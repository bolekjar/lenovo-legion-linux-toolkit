// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../LenovoLegion-PrepareBuild/UiOptions.pb.h"

#include <QObject>
#include <QVariant>

#include <vector>

namespace google::protobuf {
class FieldDescriptor;
}

namespace LenovoLegionGui {

class DataSource;

/*
 * One field of a DataSource for QML: value and presentation from the (legion.ui.field) options.
 * Options missing on the leaf are taken from the parent fields (label, unit, supported_from, hint).
 * Paths in the options are relative to the message which contains the field with the option.
 * The value is shown in displayed units (scale), references (min: "max_perf_pct") are resolved in the message,
 * so all properties are evaluated again on every change of the message.
 */
class FieldModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant     value    READ value    WRITE setValue NOTIFY changed)   // displayed units
    Q_PROPERTY(QString      path     READ path     CONSTANT)
    Q_PROPERTY(QString      label    READ label    CONSTANT)
    Q_PROPERTY(QString      unit     READ unit     CONSTANT)
    Q_PROPERTY(QString      hint     READ hint     CONSTANT)
    Q_PROPERTY(QString      tooltip  READ tooltip  CONSTANT)
    Q_PROPERTY(QString      widget   READ widget   NOTIFY changed)
    Q_PROPERTY(bool         readonly READ readonly CONSTANT)
    Q_PROPERTY(double       step     READ step     CONSTANT)
    Q_PROPERTY(QVariant     min      READ min      NOTIFY changed)
    Q_PROPERTY(QVariant     max      READ max      NOTIFY changed)
    Q_PROPERTY(QVariantList choices  READ choices  NOTIFY changed)
    Q_PROPERTY(bool         visible  READ visible  NOTIFY changed)
    Q_PROPERTY(bool         modified READ modified NOTIFY changed)   // change of the user not applied yet

public:

    /*
     * fields: descriptors along the path, the last one is the leaf
     */
    FieldModel(DataSource& source,const QString& path,std::vector<const google::protobuf::FieldDescriptor*> fields);

    QVariant     value() const;
    void         setValue(const QVariant& value);

    QString      path() const;
    QString      label() const;
    QString      unit() const;
    QString      hint() const;
    QString      tooltip() const;
    QString      widget() const;      // "switch", "slider", "spin", "combo", "radio", "label", "hidden"
    bool         readonly() const;
    double       step() const;
    QVariant     min() const;
    QVariant     max() const;
    QVariantList choices() const;     // [{text, value}]
    bool         visible() const;
    bool         modified() const;

    /*
     * Displayed value -> value of the field in the message (scale)
     */
    QVariant     toStored(const QVariant& displayed) const;

signals:

    void changed();

private:

    /*
     * Value of an option and the index of the field (in m_fields) which has it
     */
    struct Option
    {
        QString value;
        size_t  level = 0;
    };

    /*
     * Option of the leaf, or of the nearest parent field which has it (inherited options)
     */
    template<class Getter>
    Option inherited(Getter getter) const;

    /*
     * Option of the leaf only
     */
    template<class Getter>
    Option leaf(Getter getter) const;

    const legion::ui::FieldUi& leafUi() const;

    /*
     * Path of the option relative to the message which contains the field with the option
     */
    QString absolute(const QString& path,size_t level) const;

    /*
     * Number, or value of the field (leading '-' negates it), in displayed units
     */
    QVariant reference(const Option& option) const;

    /*
     * Stored value -> displayed value, rounded away from zero (the same as the widget pages, -39062 uV -> -40 mV)
     */
    QVariant display(const QVariant& stored) const;

    double   scale() const;

    bool     condition(const Option& expression) const;

private:

    DataSource&                                                 m_source;
    const QString                                               m_path;
    const std::vector<const google::protobuf::FieldDescriptor*> m_fields;
};

}
