// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Reflection.h"

#include "../LenovoLegion-PrepareBuild/UiOptions.pb.h"

#include <QRegularExpression>

#include <algorithm>

namespace LenovoLegionGui::Reflection {

using namespace google::protobuf;

namespace {

/*
 * Key of a map entry as text (keys are integers or strings)
 */
QString entryKey(const Message& entry)
{
    const FieldDescriptor* key        = entry.GetDescriptor()->map_key();
    const auto*            reflection = entry.GetReflection();

    switch(key->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_INT32:  return QString::number(reflection->GetInt32(entry,key));
    case FieldDescriptor::CPPTYPE_INT64:  return QString::number(reflection->GetInt64(entry,key));
    case FieldDescriptor::CPPTYPE_UINT32: return QString::number(reflection->GetUInt32(entry,key));
    case FieldDescriptor::CPPTYPE_UINT64: return QString::number(reflection->GetUInt64(entry,key));
    case FieldDescriptor::CPPTYPE_BOOL:   return reflection->GetBool(entry,key) ? "true" : "false";
    case FieldDescriptor::CPPTYPE_STRING: return QString::fromStdString(reflection->GetString(entry,key));
    default:                              return {};
    }
}

/*
 * Value message of the map entry with the key, nullptr if there is no such entry
 */
const Message* mapValue(const Message& message,const PathStep& step)
{
    const auto* reflection = message.GetReflection();

    for(int i = 0; i < reflection->FieldSize(message,step.field); ++i)
    {
        const Message& entry = reflection->GetRepeatedMessage(message,step.field,i);

        if(entryKey(entry) == step.key)
        {
            return &entry.GetReflection()->GetMessage(entry,entry.GetDescriptor()->map_value());
        }
    }

    return nullptr;
}

/*
 * Message which contains the leaf. Missing parent: nullptr if presence is required, the default instance otherwise.
 */
const Message* parentOf(const Message& message,const FieldPath& path,bool requirePresence)
{
    const Message* current = &message;

    for(size_t i = 0; i + 1 < path.size(); ++i)
    {
        const PathStep& step = path[i];

        if(step.mapKey)
        {
            const Message* value = mapValue(*current,step);

            if(value == nullptr)
            {
                if(requirePresence)
                {
                    return nullptr;
                }

                value = MessageFactory::generated_factory()->GetPrototype(step.field->message_type()->map_value()->message_type());
            }

            current = value;
            continue;
        }

        if(step.index)
        {
            if(step.position >= current->GetReflection()->FieldSize(*current,step.field))
            {
                if(requirePresence)
                {
                    return nullptr;
                }

                current = MessageFactory::generated_factory()->GetPrototype(step.field->message_type());
                continue;
            }

            current = &current->GetReflection()->GetRepeatedMessage(*current,step.field,step.position);
            continue;
        }

        if(requirePresence && !current->GetReflection()->HasField(*current,step.field))
        {
            return nullptr;
        }

        current = &current->GetReflection()->GetMessage(*current,step.field);
    }

    return current;
}

bool throughMap(const FieldPath& path)
{
    return std::any_of(path.begin(),path.end(),[](const PathStep& step) { return step.mapKey; });
}

Message* mutableParentOf(Message& message,const FieldPath& path)
{
    Message* current = &message;

    for(size_t i = 0; i + 1 < path.size(); ++i)
    {
        const auto* reflection = current->GetReflection();

        if(path[i].index)
        {
            /*
             * Elements before the index are empty (nothing is changed for them)
             */
            while(reflection->FieldSize(*current,path[i].field) <= path[i].position)
            {
                reflection->AddMessage(current,path[i].field);
            }

            current = reflection->MutableRepeatedMessage(current,path[i].field,path[i].position);
            continue;
        }

        current = reflection->MutableMessage(current,path[i].field);
    }

    return current;
}

}

FieldPath resolve(const Descriptor *descriptor, const QString &path)
{
    static const QRegularExpression keyPattern(R"(^(\w+)\[([^\]]+)\]$)");

    FieldPath steps;

    const QStringList parts = path.split('.');

    for(qsizetype i = 0; i < parts.size(); ++i)
    {
        if(descriptor == nullptr)
        {
            return {};
        }

        PathStep   step;
        QString    name  = parts[i];
        const auto match = keyPattern.match(name);

        if(match.hasMatch())
        {
            name         = match.captured(1);
            step.mapKey  = true;
            step.key     = match.captured(2).trimmed();
        }

        step.field = descriptor->FindFieldByName(name.toStdString());

        if(step.field == nullptr)
        {
            return {};
        }

        const bool last = i + 1 == parts.size();

        if(step.mapKey && !step.field->is_map())
        {
            /*
             * [n] of a repeated message field: element by index
             */
            bool      ok       = false;
            const int position = step.key.toInt(&ok);

            if(!ok || position < 0 || !step.field->is_repeated() || last || step.field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
            {
                return {};
            }

            step.mapKey   = false;
            step.index    = true;
            step.position = position;
            step.key.clear();

            descriptor = step.field->message_type();
        }
        else if(step.mapKey)
        {
            // Map with a message value, followed by a field of the value
            if(last || step.field->message_type()->map_value()->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
            {
                return {};
            }

            descriptor = step.field->message_type()->map_value()->message_type();
        }
        else
        {
            // Repeated field (also map) only as the leaf
            if(step.field->is_repeated() && !last)
            {
                return {};
            }

            descriptor = step.field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE ? step.field->message_type() : nullptr;
        }

        steps.push_back(step);
    }

    return steps;
}

std::vector<const FieldDescriptor *> descriptors(const FieldPath &path)
{
    std::vector<const FieldDescriptor*> fields;

    for(const auto& step : path)
    {
        fields.push_back(step.field);
    }

    return fields;
}

void leafPaths(const Descriptor *descriptor, const QString &prefix, QStringList &paths)
{
    for(int i = 0; i < descriptor->field_count(); ++i)
    {
        const FieldDescriptor* field = descriptor->field(i);
        const QString          path  = QString(prefix).append(QString::fromStdString(std::string(field->name())));

        if(field->options().GetExtension(legion::ui::field).widget() == legion::ui::HIDDEN || field->is_repeated())
        {
            continue;
        }

        if(field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE)
        {
            leafPaths(field->message_type(),QString(path).append('.'),paths);
        }
        else
        {
            paths.append(path);
        }
    }
}

bool has(const Message &message, const FieldPath &path)
{
    if(path.empty())
    {
        return false;
    }

    const Message* parent = parentOf(message,path,true);

    if(parent == nullptr)
    {
        return false;
    }

    const FieldDescriptor* leaf = path.back().field;

    if(leaf->is_repeated())
    {
        return parent->GetReflection()->FieldSize(*parent,leaf) > 0;
    }

    return !leaf->has_presence() || parent->GetReflection()->HasField(*parent,leaf);
}

QVariant read(const Message &message, const FieldPath &path)
{
    if(path.empty() || path.back().field->is_repeated())
    {
        return {};
    }

    const Message*          parent     = parentOf(message,path,false);
    const FieldDescriptor*  leaf       = path.back().field;
    const auto*             reflection = parent->GetReflection();

    switch(leaf->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_BOOL:   return reflection->GetBool(*parent,leaf);
    case FieldDescriptor::CPPTYPE_INT32:  return static_cast<qlonglong>(reflection->GetInt32(*parent,leaf));
    case FieldDescriptor::CPPTYPE_INT64:  return static_cast<qlonglong>(reflection->GetInt64(*parent,leaf));
    case FieldDescriptor::CPPTYPE_UINT32: return static_cast<qulonglong>(reflection->GetUInt32(*parent,leaf));
    case FieldDescriptor::CPPTYPE_UINT64: return static_cast<qulonglong>(reflection->GetUInt64(*parent,leaf));
    case FieldDescriptor::CPPTYPE_FLOAT:  return static_cast<double>(reflection->GetFloat(*parent,leaf));
    case FieldDescriptor::CPPTYPE_DOUBLE: return reflection->GetDouble(*parent,leaf);
    case FieldDescriptor::CPPTYPE_STRING: return QString::fromStdString(reflection->GetString(*parent,leaf)).trimmed();
    case FieldDescriptor::CPPTYPE_ENUM:   return reflection->GetEnumValue(*parent,leaf);
    default:                              return {};
    }
}

void write(Message &message, const FieldPath &path, const QVariant &value)
{
    if(path.empty() || path.back().field->is_repeated() || throughMap(path))
    {
        return;
    }

    Message*                parent     = mutableParentOf(message,path);
    const FieldDescriptor*  leaf       = path.back().field;
    const auto*             reflection = parent->GetReflection();

    switch(leaf->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_BOOL:   reflection->SetBool(parent,leaf,value.toBool());                                   break;
    case FieldDescriptor::CPPTYPE_INT32:  reflection->SetInt32(parent,leaf,value.toInt());                                   break;
    case FieldDescriptor::CPPTYPE_INT64:  reflection->SetInt64(parent,leaf,value.toLongLong());                              break;
    case FieldDescriptor::CPPTYPE_UINT32: reflection->SetUInt32(parent,leaf,value.toUInt());                                 break;
    case FieldDescriptor::CPPTYPE_UINT64: reflection->SetUInt64(parent,leaf,value.toULongLong());                            break;
    case FieldDescriptor::CPPTYPE_FLOAT:  reflection->SetFloat(parent,leaf,value.toFloat());                                 break;
    case FieldDescriptor::CPPTYPE_DOUBLE: reflection->SetDouble(parent,leaf,value.toDouble());                               break;
    case FieldDescriptor::CPPTYPE_STRING: reflection->SetString(parent,leaf,value.toString().toStdString());                 break;
    case FieldDescriptor::CPPTYPE_ENUM:   reflection->SetEnumValue(parent,leaf,value.toInt());                               break;
    default:                                                                                                                 break;
    }
}

void clear(Message &message, const FieldPath &path)
{
    if(path.empty() || throughMap(path))
    {
        return;
    }

    /*
     * Messages along the path, the leaf is cleared in the last one
     */
    std::vector<Message*> messages { &message };

    for(size_t i = 0; i + 1 < path.size(); ++i)
    {
        Message*    current    = messages.back();
        const auto* reflection = current->GetReflection();

        if(path[i].index)
        {
            if(path[i].position >= reflection->FieldSize(*current,path[i].field))
            {
                return;
            }

            messages.push_back(reflection->MutableRepeatedMessage(current,path[i].field,path[i].position));
            continue;
        }

        if(!reflection->HasField(*current,path[i].field))
        {
            return;
        }

        messages.push_back(reflection->MutableMessage(current,path[i].field));
    }

    messages.back()->GetReflection()->ClearField(messages.back(),path.back().field);

    /*
     * Empty parents are removed: messages are cleared, empty trailing elements of repeated fields are removed
     */
    for(size_t i = messages.size() - 1; i > 0; --i)
    {
        if(messages[i]->ByteSizeLong() > 0)
        {
            break;
        }

        Message*    parent     = messages[i - 1];
        const auto* reflection = parent->GetReflection();

        if(path[i - 1].index)
        {
            while(reflection->FieldSize(*parent,path[i - 1].field) > 0 &&
                  reflection->GetRepeatedMessage(*parent,path[i - 1].field,reflection->FieldSize(*parent,path[i - 1].field) - 1).ByteSizeLong() == 0)
            {
                reflection->RemoveLast(parent,path[i - 1].field);
            }

            if(reflection->FieldSize(*parent,path[i - 1].field) > 0)
            {
                break;
            }

            continue;
        }

        reflection->ClearField(parent,path[i - 1].field);
    }
}

QVariantList repeatedValues(const Message &message, const FieldPath &path)
{
    QVariantList values;

    if(path.empty() || !path.back().field->is_repeated() || path.back().field->is_map())
    {
        return values;
    }

    const Message* parent = parentOf(message,path,true);

    if(parent == nullptr)
    {
        return values;
    }

    const auto*            reflection = parent->GetReflection();
    const FieldDescriptor* leaf       = path.back().field;

    for(int i = 0; i < reflection->FieldSize(*parent,leaf); ++i)
    {
        switch(leaf->cpp_type())
        {
        case FieldDescriptor::CPPTYPE_STRING: values.append(QString::fromStdString(reflection->GetRepeatedString(*parent,leaf,i))); break;
        case FieldDescriptor::CPPTYPE_ENUM:   values.append(reflection->GetRepeatedEnumValue(*parent,leaf,i));                   break;
        case FieldDescriptor::CPPTYPE_INT32:  values.append(static_cast<qlonglong>(reflection->GetRepeatedInt32(*parent,leaf,i)));   break;
        case FieldDescriptor::CPPTYPE_INT64:  values.append(static_cast<qlonglong>(reflection->GetRepeatedInt64(*parent,leaf,i)));   break;
        case FieldDescriptor::CPPTYPE_UINT32: values.append(static_cast<qulonglong>(reflection->GetRepeatedUInt32(*parent,leaf,i))); break;
        case FieldDescriptor::CPPTYPE_UINT64: values.append(static_cast<qulonglong>(reflection->GetRepeatedUInt64(*parent,leaf,i))); break;
        default:                                                                                                                 break;
        }
    }

    return values;
}

int count(const Message &message, const FieldPath &path)
{
    if(path.empty() || !path.back().field->is_repeated())
    {
        return 0;
    }

    const Message* parent = parentOf(message,path,true);

    return parent == nullptr ? 0 : parent->GetReflection()->FieldSize(*parent,path.back().field);
}


QVariant toVariant(const Message &message, const FieldDescriptor *field)
{
    const auto* reflection = message.GetReflection();

    auto scalar = [&](int index) -> QVariant {
        const bool repeated = field->is_repeated();

        switch(field->cpp_type())
        {
        case FieldDescriptor::CPPTYPE_BOOL:    return repeated ? reflection->GetRepeatedBool(message,field,index)   : reflection->GetBool(message,field);
        case FieldDescriptor::CPPTYPE_INT32:   return static_cast<qlonglong>(repeated ? reflection->GetRepeatedInt32(message,field,index)   : reflection->GetInt32(message,field));
        case FieldDescriptor::CPPTYPE_INT64:   return static_cast<qlonglong>(repeated ? reflection->GetRepeatedInt64(message,field,index)   : reflection->GetInt64(message,field));
        case FieldDescriptor::CPPTYPE_UINT32:  return static_cast<qulonglong>(repeated ? reflection->GetRepeatedUInt32(message,field,index) : reflection->GetUInt32(message,field));
        case FieldDescriptor::CPPTYPE_UINT64:  return static_cast<qulonglong>(repeated ? reflection->GetRepeatedUInt64(message,field,index) : reflection->GetUInt64(message,field));
        case FieldDescriptor::CPPTYPE_FLOAT:   return static_cast<double>(repeated ? reflection->GetRepeatedFloat(message,field,index) : reflection->GetFloat(message,field));
        case FieldDescriptor::CPPTYPE_DOUBLE:  return repeated ? reflection->GetRepeatedDouble(message,field,index) : reflection->GetDouble(message,field);
        case FieldDescriptor::CPPTYPE_STRING:  return QString::fromStdString(repeated ? reflection->GetRepeatedString(message,field,index) : reflection->GetString(message,field));
        case FieldDescriptor::CPPTYPE_ENUM:    return repeated ? reflection->GetRepeatedEnumValue(message,field,index) : reflection->GetEnumValue(message,field);
        case FieldDescriptor::CPPTYPE_MESSAGE: return toVariant(repeated ? reflection->GetRepeatedMessage(message,field,index) : reflection->GetMessage(message,field));
        }

        return {};
    };

    if(field->is_map())
    {
        return {};
    }

    if(field->is_repeated())
    {
        QVariantList values;

        for(int i = 0; i < reflection->FieldSize(message,field); ++i)
        {
            values.append(scalar(i));
        }

        return values;
    }

    return scalar(0);
}

QVariantMap toVariant(const Message &message)
{
    QVariantMap values;
    const Descriptor* descriptor = message.GetDescriptor();

    for(int i = 0; i < descriptor->field_count(); ++i)
    {
        const FieldDescriptor* field = descriptor->field(i);

        if(!field->is_map())
        {
            values.insert(QString::fromStdString(std::string(field->name())),toVariant(message,field));
        }
    }

    return values;
}

void fromVariant(Message &message, const QVariantMap &values, QStringList *unknown)
{
    const Descriptor* descriptor = message.GetDescriptor();
    const auto*       reflection = message.GetReflection();

    // Enum by number or by its name
    auto enumNumber = [](const FieldDescriptor* field,const QVariant& value) {
        if(value.typeId() == QMetaType::QString)
        {
            const EnumValueDescriptor* enumValue = field->enum_type()->FindValueByName(value.toString().toStdString());
            return enumValue != nullptr ? enumValue->number() : 0;
        }

        return value.toInt();
    };

    for(auto it = values.cbegin(); it != values.cend(); ++it)
    {
        const FieldDescriptor* field = descriptor->FindFieldByName(it.key().toStdString());

        if(field == nullptr || field->is_map())
        {
            if(unknown != nullptr)
            {
                unknown->append(it.key());
            }

            continue;
        }

        const QVariant& value = it.value();

        if(field->is_repeated())
        {
            reflection->ClearField(&message,field);

            for(const QVariant& element : value.toList())
            {
                switch(field->cpp_type())
                {
                case FieldDescriptor::CPPTYPE_BOOL:    reflection->AddBool(&message,field,element.toBool());                       break;
                case FieldDescriptor::CPPTYPE_INT32:   reflection->AddInt32(&message,field,element.toInt());                       break;
                case FieldDescriptor::CPPTYPE_INT64:   reflection->AddInt64(&message,field,element.toLongLong());                  break;
                case FieldDescriptor::CPPTYPE_UINT32:  reflection->AddUInt32(&message,field,static_cast<quint32>(element.toLongLong())); break;
                case FieldDescriptor::CPPTYPE_UINT64:  reflection->AddUInt64(&message,field,element.toULongLong());                break;
                case FieldDescriptor::CPPTYPE_FLOAT:   reflection->AddFloat(&message,field,element.toFloat());                     break;
                case FieldDescriptor::CPPTYPE_DOUBLE:  reflection->AddDouble(&message,field,element.toDouble());                   break;
                case FieldDescriptor::CPPTYPE_STRING:  reflection->AddString(&message,field,element.toString().toStdString());     break;
                case FieldDescriptor::CPPTYPE_ENUM:    reflection->AddEnumValue(&message,field,enumNumber(field,element));         break;
                case FieldDescriptor::CPPTYPE_MESSAGE: fromVariant(*reflection->AddMessage(&message,field),element.toMap(),unknown); break;
                }
            }

            continue;
        }

        switch(field->cpp_type())
        {
        case FieldDescriptor::CPPTYPE_BOOL:    reflection->SetBool(&message,field,value.toBool());                         break;
        case FieldDescriptor::CPPTYPE_INT32:   reflection->SetInt32(&message,field,value.toInt());                         break;
        case FieldDescriptor::CPPTYPE_INT64:   reflection->SetInt64(&message,field,value.toLongLong());                    break;
        case FieldDescriptor::CPPTYPE_UINT32:  reflection->SetUInt32(&message,field,static_cast<quint32>(value.toLongLong())); break;
        case FieldDescriptor::CPPTYPE_UINT64:  reflection->SetUInt64(&message,field,value.toULongLong());                  break;
        case FieldDescriptor::CPPTYPE_FLOAT:   reflection->SetFloat(&message,field,value.toFloat());                       break;
        case FieldDescriptor::CPPTYPE_DOUBLE:  reflection->SetDouble(&message,field,value.toDouble());                     break;
        case FieldDescriptor::CPPTYPE_STRING:  reflection->SetString(&message,field,value.toString().toStdString());       break;
        case FieldDescriptor::CPPTYPE_ENUM:    reflection->SetEnumValue(&message,field,enumNumber(field,value));           break;
        case FieldDescriptor::CPPTYPE_MESSAGE: fromVariant(*reflection->MutableMessage(&message,field),value.toMap(),unknown); break;
        }
    }
}

}
