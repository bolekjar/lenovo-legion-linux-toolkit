// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "FieldModel.h"
#include "DataSource.h"

#include <google/protobuf/descriptor.h>

#include <QRegularExpression>

#include <cmath>

namespace LenovoLegionGui {

using namespace google::protobuf;

namespace {

QString toQString(std::string_view value)
{
    return QString::fromUtf8(value.data(),static_cast<qsizetype>(value.size()));
}

}

FieldModel::FieldModel(DataSource &source, const QString &path, std::vector<const FieldDescriptor *> fields) :
    m_source(source),
    m_path(path),
    m_fields(std::move(fields))
{
    connect(&m_source,&DataSource::messageChanged,this,&FieldModel::changed);
}

QVariant FieldModel::value() const
{
    /*
     * value_from: displayed value from another field (PowerProfile: thermal_mode), this field is written
     */
    const Option valueFrom = leaf([](const legion::ui::FieldUi& ui) { return ui.value_from(); });

    if(!valueFrom.value.isEmpty() && !m_source.modified(m_path))
    {
        return display(m_source.value(absolute(valueFrom.value,valueFrom.level)));
    }

    return display(m_source.value(m_path));
}

void FieldModel::setValue(const QVariant &value)
{
    if(readonly())
    {
        return;
    }

    const bool valueFrom = !leafUi().value_from().empty();

    /*
     * Back to the displayed value of the daemon (scaled value differs from the stored one, e.g. -40 mV and -39062 uV): no change
     */
    if(!valueFrom && scale() != 1.0 && m_source.present(m_path) && display(m_source.daemonValue(m_path)) == display(toStored(value)))
    {
        m_source.clearChange(m_path);
        return;
    }

    /*
     * value_from: always written, the written field may already have the value (smart fan) while the shown one differs (thermal mode)
     */
    m_source.setValue(m_path,toStored(value),valueFrom);
}

QString FieldModel::path() const
{
    return m_path;
}

QString FieldModel::label() const
{
    const QString label = inherited([](const legion::ui::FieldUi& ui) { return ui.label(); }).value;

    return label.isEmpty() ? toQString(m_fields.back()->name()) : label;
}

QString FieldModel::unit() const
{
    return inherited([](const legion::ui::FieldUi& ui) { return ui.unit(); }).value;
}

QString FieldModel::hint() const
{
    return inherited([](const legion::ui::FieldUi& ui) { return ui.hint(); }).value;
}

QString FieldModel::tooltip() const
{
    return inherited([](const legion::ui::FieldUi& ui) { return ui.tooltip(); }).value;
}

QString FieldModel::widget() const
{
    /*
     * Number with allowed values from the data (steps of a power limit) is a combo box
     */
    if((leafUi().widget() == legion::ui::SLIDER || leafUi().widget() == legion::ui::SPIN) && !leafUi().choices_from().empty() && !choices().isEmpty())
    {
        return "combo";
    }

    switch(leafUi().widget())
    {
    case legion::ui::SWITCH: return "switch";
    case legion::ui::SLIDER: return "slider";
    case legion::ui::SPIN:   return "spin";
    case legion::ui::COMBO:  return "combo";
    case legion::ui::RADIO:  return "radio";
    case legion::ui::LABEL:  return "label";
    case legion::ui::HIDDEN: return "hidden";
    default:                 break;
    }

    /*
     * AUTO: by the type of the field
     */
    const FieldDescriptor* leafField = m_fields.back();

    if(!leafUi().choices_from().empty() || leafField->cpp_type() == FieldDescriptor::CPPTYPE_ENUM)
    {
        return "combo";
    }

    switch(leafField->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_BOOL:   return "switch";
    case FieldDescriptor::CPPTYPE_STRING: return "label";
    default:                              return "spin";
    }
}

bool FieldModel::readonly() const
{
    return leafUi().readonly() || widget() == "label";
}

double FieldModel::step() const
{
    return leafUi().step() > 0.0 ? leafUi().step() : 1.0;
}

QVariant FieldModel::min() const
{
    return reference(leaf([](const legion::ui::FieldUi& ui) { return ui.min(); }));
}

QVariant FieldModel::max() const
{
    return reference(leaf([](const legion::ui::FieldUi& ui) { return ui.max(); }));
}

QVariantList FieldModel::choices() const
{
    QVariantList choices;

    const FieldDescriptor* leafField = m_fields.back();
    const EnumDescriptor*  enumType  = leafField->cpp_type() == FieldDescriptor::CPPTYPE_ENUM ? leafField->enum_type() : nullptr;

    auto enumChoice = [](const EnumValueDescriptor* value) {
        const std::string label = value->options().GetExtension(legion::ui::value_label);
        const std::string color   = value->options().GetExtension(legion::ui::value_color);
        const std::string tooltip = value->options().GetExtension(legion::ui::value_tooltip);

        QVariantMap choice{
            {"text",  label.empty() ? toQString(value->name()) : QString::fromStdString(label)},
            {"value", value->number()}
        };

        if(!color.empty())
        {
            choice.insert("color",QString::fromStdString(color));
        }

        if(!tooltip.empty())
        {
            choice.insert("tooltip",QString::fromStdString(tooltip));
        }

        return choice;
    };

    /*
     * Allowed values from the data (settable values, supported profiles reported by the daemon)
     */
    const Option choicesFrom = leaf([](const legion::ui::FieldUi& ui) { return ui.choices_from(); });

    if(!choicesFrom.value.isEmpty())
    {
        for(const auto& value : m_source.repeatedValues(absolute(choicesFrom.value,choicesFrom.level)))
        {
            const EnumValueDescriptor* enumValue = enumType != nullptr ? enumType->FindValueByNumber(value.toInt()) : nullptr;

            if(enumValue != nullptr)
            {
                choices.append(enumChoice(enumValue));
            }
            else
            {
                choices.append(QVariantMap{{"text",value.toString()},{"value",value}});
            }
        }

        return choices;
    }

    if(enumType != nullptr)
    {
        for(int i = 0; i < enumType->value_count(); ++i)
        {
            choices.append(enumChoice(enumType->value(i)));
        }
    }

    return choices;
}

bool FieldModel::visible() const
{
    if(widget() == "hidden")
    {
        return false;
    }

    /*
     * Not sent by the daemon = not supported by the hardware
     */
    const Option valueFrom = leaf([](const legion::ui::FieldUi& ui) { return ui.value_from(); });

    if(!m_source.present(m_path) && (valueFrom.value.isEmpty() || !m_source.present(absolute(valueFrom.value,valueFrom.level))))
    {
        return false;
    }

    const Option supportedFrom = inherited([](const legion::ui::FieldUi& ui) { return ui.supported_from(); });

    if(!supportedFrom.value.isEmpty() && !m_source.value(absolute(supportedFrom.value,supportedFrom.level)).toBool())
    {
        return false;
    }

    return condition(leaf([](const legion::ui::FieldUi& ui) { return ui.visible_when(); }));
}

bool FieldModel::modified() const
{
    return m_source.modified(m_path);
}

QVariant FieldModel::toStored(const QVariant &displayed) const
{
    if(scale() == 1.0 || !displayed.canConvert<double>())
    {
        return displayed;
    }

    return static_cast<qlonglong>(std::llround(displayed.toDouble() / scale()));
}

template<class Getter>
FieldModel::Option FieldModel::inherited(Getter getter) const
{
    for(size_t level = m_fields.size(); level-- > 0;)
    {
        const std::string value(getter(m_fields[level]->options().GetExtension(legion::ui::field)));

        if(!value.empty())
        {
            return { QString::fromStdString(value), level };
        }
    }

    return {};
}

template<class Getter>
FieldModel::Option FieldModel::leaf(Getter getter) const
{
    return { QString::fromStdString(std::string(getter(leafUi()))), m_fields.size() - 1 };
}

const legion::ui::FieldUi &FieldModel::leafUi() const
{
    return m_fields.back()->options().GetExtension(legion::ui::field);
}

QString FieldModel::absolute(const QString &path, size_t level) const
{
    /*
     * Parent message of the field with the option = first `level` components of the path
     */
    const QStringList parts = m_path.split('.').mid(0,static_cast<qsizetype>(level));

    return parts.isEmpty() ? path : parts.join('.').append('.').append(path);
}

QVariant FieldModel::reference(const Option &option) const
{
    if(option.value.isEmpty())
    {
        return {};
    }

    bool         ok     = false;
    const double number = option.value.toDouble(&ok);

    if(ok)
    {
        return number;
    }

    /*
     * Field of the message, leading '-' negates it (min: "-max_undervolt")
     */
    const bool    negative = option.value.startsWith('-');
    const QString path     = absolute(negative ? option.value.mid(1) : option.value,option.level);
    const QVariant value   = display(m_source.value(path));

    if(!value.isValid())
    {
        return {};
    }

    return negative ? QVariant(-value.toDouble()) : value;
}

QVariant FieldModel::display(const QVariant &stored) const
{
    if(scale() == 1.0 || !stored.isValid() || !stored.canConvert<double>())
    {
        return stored;
    }

    const double scaled = stored.toDouble() * scale();

    /*
     * Away from zero, a small tolerance keeps exact values (-40000 uV -> -40 mV)
     */
    return static_cast<qlonglong>(scaled > 0 ? std::ceil(scaled - 1e-9) : std::floor(scaled + 1e-9));
}

double FieldModel::scale() const
{
    return leafUi().scale() != 0.0 ? leafUi().scale() : 1.0;
}

bool FieldModel::condition(const Option &expression) const
{
    if(expression.value.trimmed().isEmpty())
    {
        return true;
    }

    /*
     * "path == value" or "path != value"
     */
    static const QRegularExpression pattern(R"(^\s*([\w.]+)\s*(==|!=)\s*(.*?)\s*$)");

    const auto match = pattern.match(expression.value);

    if(!match.hasMatch())
    {
        return true;
    }

    const bool equal = m_source.value(absolute(match.captured(1),expression.level)).toString() == match.captured(3);

    return match.captured(2) == "==" ? equal : !equal;
}

}
