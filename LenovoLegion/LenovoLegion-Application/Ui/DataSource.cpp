// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataSource.h"
#include "FieldModel.h"
#include "Reflection.h"

#include "../DataProvider.h"

#include "../LenovoLegion-PrepareBuild/MessageRegistry.pb.h"
#include "../LenovoLegion-PrepareBuild/UiOptions.pb.h"

#include <Core/LoggerHolder.h>

#include <QtConcurrent/QtConcurrentRun>

#include <google/protobuf/descriptor.h>

namespace LenovoLegionGui {

using namespace google::protobuf;

namespace {

/*
 * Message fields with the option atomic which have a change are sent as a whole:
 * the state from the daemon with the changes of the user (the kernel accepts only the whole fan curve)
 */
void completeAtomic(const Message& state,Message& changes)
{
    const Descriptor* descriptor = changes.GetDescriptor();
    const auto*       reflection = changes.GetReflection();

    for(int i = 0; i < descriptor->field_count(); ++i)
    {
        const FieldDescriptor* field = descriptor->field(i);

        if(field->is_repeated() || field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE || !reflection->HasField(changes,field))
        {
            continue;
        }

        const Message& stateField = state.GetReflection()->GetMessage(state,field);
        Message*       changed    = reflection->MutableMessage(&changes,field);

        if(field->options().GetExtension(legion::ui::field).atomic())
        {
            std::unique_ptr<Message> whole(stateField.New());

            whole->CopyFrom(stateField);
            whole->MergeFrom(*changed);
            changed->CopyFrom(*whole);
        }
        else
        {
            completeAtomic(stateField,*changed);
        }
    }
}

}

DataProvider*     DataSource::s_dataProvider = nullptr;
QSet<DataSource*> DataSource::s_dataSources;

DataSource::DataSource(QObject *parent) :
    QObject(parent),
    m_dataTypeValue(-1),
    m_autoApply(false),
    m_timer(new QTimer(this))
{
    connect(m_timer,&QTimer::timeout,this,&DataSource::reloadAsync);

    s_dataSources.insert(this);
}

DataSource::~DataSource()
{
    s_dataSources.remove(this);
}

void DataSource::setDataProvider(DataProvider *dataProvider)
{
    s_dataProvider = dataProvider;
}

void DataSource::reloadAll()
{
    for(auto* source : std::as_const(s_dataSources))
    {
        source->reload();
    }
}

QString DataSource::dataType() const
{
    return m_dataType;
}

void DataSource::setDataType(const QString &dataType)
{
    if(dataType == m_dataType)
    {
        return;
    }

    m_dataType      = dataType;
    m_dataTypeValue = -1;
    m_available     = false;
    m_platform      = "generic";

    for(auto& [path,field] : m_fields)
    {
        delete field;
    }

    m_fields.clear();
    m_message.reset();
    m_changes.reset();

    /*
     * Message of the data type from (legion.ui.message) of the DataType value
     */
    const EnumValueDescriptor* value = legion::messages::DataType_descriptor()->FindValueByName(dataType.toStdString());

    if(value == nullptr)
    {
        setLastError(QString("Unknown data type ").append(dataType));
    }
    else
    {
        const std::string  name       = value->options().GetExtension(legion::ui::message);
        const Descriptor*  descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(name);

        if(descriptor == nullptr)
        {
            setLastError(QString("Data type ").append(dataType).append(" has no message (legion.ui.message)"));
        }
        else
        {
            const Message* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);

            m_dataTypeValue = value->number();
            m_platform      = value->options().GetExtension(legion::ui::platform) == legion::ui::PLATFORM_LENOVO ? "lenovo" : "generic";
            m_message.reset(prototype->New());
            m_changes.reset(prototype->New());
        }
    }

    emit dataTypeChanged();

    reload();
}

bool DataSource::autoApply() const
{
    return m_autoApply;
}

void DataSource::setAutoApply(bool autoApply)
{
    if(autoApply != m_autoApply)
    {
        m_autoApply = autoApply;
        emit autoApplyChanged();
    }
}

bool DataSource::dirty() const
{
    return m_changes != nullptr && m_changes->ByteSizeLong() > 0;
}

bool DataSource::valid() const
{
    return m_message != nullptr;
}

bool DataSource::available() const
{
    return m_available;
}

QString DataSource::platform() const
{
    return m_platform;
}

QString DataSource::lastError() const
{
    return m_lastError;
}

int DataSource::revision() const
{
    return m_revision;
}

void DataSource::reload()
{
    if(s_dataProvider == nullptr || m_message == nullptr)
    {
        return;
    }

    try {
        parse(s_dataProvider->getData(static_cast<quint8>(m_dataTypeValue)));
    }
    catch(const std::exception& ex)
    {
        LOG_E(QString("DataSource ").append(m_dataType).append(": reload failed: ").append(ex.what()));
        setLastError(ex.what());
        m_available = false;
    }

    ++m_revision;
    emit messageChanged();
}

void DataSource::reloadAsync()
{
    if(s_dataProvider == nullptr || m_message == nullptr || m_reloading)
    {
        return;
    }

    m_reloading = true;

    const quint8 dataType = static_cast<quint8>(m_dataTypeValue);

    /*
     * Context object: the continuation is not called when the data source is deleted
     */
    QtConcurrent::run([dataType]() -> std::pair<bool,QByteArray> {
        try {
            return { true, s_dataProvider->getData(dataType) };
        }
        catch(const std::exception& ex)
        {
            return { false, QByteArray(ex.what()) };
        }
    }).then(this,[this](const std::pair<bool,QByteArray>& result) {
        m_reloading = false;

        if(result.first)
        {
            parse(result.second);
        }
        else
        {
            LOG_E(QString("DataSource ").append(m_dataType).append(": periodic reload failed: ").append(QString::fromUtf8(result.second)));
            setLastError(QString::fromUtf8(result.second));
            m_available = false;
        }

        ++m_revision;
        emit messageChanged();
    });
}

void DataSource::parse(const QByteArray &data)
{
    m_available = m_message->ParseFromArray(data.constData(),static_cast<int>(data.size()));

    if(!m_available)
    {
        setLastError("Invalid data from the daemon");
    }
}

int DataSource::interval() const
{
    return m_timer->interval();
}

void DataSource::setInterval(int interval)
{
    if(interval == m_timer->interval() && m_timer->isActive() == (interval > 0))
    {
        return;
    }

    m_timer->stop();
    m_timer->setInterval(interval);

    if(interval > 0)
    {
        m_timer->start();
    }

    emit intervalChanged();
}

void DataSource::apply()
{
    if(s_dataProvider == nullptr || !dirty())
    {
        return;
    }

    completeAtomic(*m_message,*m_changes);

    std::string data;

    if(!m_changes->SerializeToString(&data))
    {
        setLastError("Serialize of the changes failed");
        return;
    }

    /*
     * Changes are sent once, the state is read again (also after an error, the real state is shown)
     */
    m_changes->Clear();

    try {
        s_dataProvider->setData(static_cast<quint8>(m_dataTypeValue),QByteArray(data.data(),static_cast<qsizetype>(data.size())));
        setLastError({});
    }
    catch(const std::exception& ex)
    {
        LOG_E(QString("DataSource ").append(m_dataType).append(": apply failed: ").append(ex.what()));
        setLastError(ex.what());
    }

    /*
     * Other sources of the same data type (e.g. power profile panel and power page) show the new state too
     */
    for(auto* source : std::as_const(s_dataSources))
    {
        if(source == this || source->m_dataTypeValue == m_dataTypeValue)
        {
            source->reload();
        }
    }
}

void DataSource::applyValues(const QVariantMap &values)
{
    if(m_message == nullptr)
    {
        return;
    }

    m_changes->Clear();

    for(auto it = values.cbegin(); it != values.cend(); ++it)
    {
        FieldModel* model = field(it.key());

        if(model == nullptr)
        {
            LOG_W(QString("DataSource ").append(m_dataType).append(": preset has unknown field ").append(it.key()));
            continue;
        }

        Reflection::write(*m_changes,Reflection::resolve(m_message->GetDescriptor(),it.key()),model->toStored(it.value()));
    }

    if(dirty())
    {
        apply();
    }
    else
    {
        ++m_revision;
        emit messageChanged();
    }
}

void DataSource::setValues(const QVariantMap &values)
{
    beginChanges();

    for(auto it = values.cbegin(); it != values.cend(); ++it)
    {
        FieldModel* model = field(it.key());

        if(model == nullptr)
        {
            LOG_W(QString("DataSource ").append(m_dataType).append(": preset has unknown field ").append(it.key()));
            continue;
        }

        /*
         * Not supported fields (e.g. not supported voltage planes) are not filled in
         */
        if(model->visible())
        {
            model->setValue(it.value());
        }
    }

    endChanges();
}

void DataSource::cancel()
{
    if(m_changes != nullptr)
    {
        m_changes->Clear();
        ++m_revision;
        emit messageChanged();
    }
}

QStringList DataSource::fieldPaths() const
{
    QStringList paths;

    if(m_message != nullptr)
    {
        Reflection::leafPaths(m_message->GetDescriptor(),{},paths);
    }

    return paths;
}

FieldModel *DataSource::field(const QString &path)
{
    const auto it = m_fields.find(path);

    if(it != m_fields.end())
    {
        return it->second;
    }

    if(m_message == nullptr)
    {
        return nullptr;
    }

    auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    if(fields.empty())
    {
        LOG_W(QString("DataSource ").append(m_dataType).append(": unknown field ").append(path));
        return nullptr;
    }

    auto* model = new FieldModel(*this,path,Reflection::descriptors(fields));

    model->setParent(this);
    m_fields.emplace(path,model);

    return model;
}

QVariant DataSource::value(const QString &path) const
{
    if(m_message == nullptr)
    {
        return {};
    }

    const auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    if(fields.empty())
    {
        return {};
    }

    if(Reflection::has(*m_changes,fields))
    {
        return Reflection::read(*m_changes,fields);
    }

    return Reflection::read(*m_message,fields);
}

bool DataSource::present(const QString &path) const
{
    if(m_message == nullptr)
    {
        return false;
    }

    const auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    return !fields.empty() && Reflection::has(*m_message,fields);
}

QVariant DataSource::daemonValue(const QString &path) const
{
    if(m_message == nullptr)
    {
        return {};
    }

    const auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    return fields.empty() ? QVariant() : Reflection::read(*m_message,fields);
}

void DataSource::clearChange(const QString &path)
{
    if(m_changes == nullptr)
    {
        return;
    }

    const auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    if(!fields.empty() && Reflection::has(*m_changes,fields))
    {
        Reflection::clear(*m_changes,fields);

        if(m_batchDepth > 0)
        {
            m_batchChanged = true;
            return;
        }

        ++m_revision;
        emit messageChanged();
    }
}

void DataSource::setValue(const QString &path, const QVariant &value, bool force)
{
    if(m_message == nullptr)
    {
        return;
    }

    const auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    if(fields.empty())
    {
        return;
    }

    /*
     * Value in the type of the field (QML sends int for uint32, ...)
     */
    std::unique_ptr<Message> probe(m_message->New());

    Reflection::write(*probe,fields,value);

    const QVariant normalized = Reflection::read(*probe,fields);

    if(!force && this->value(path) == normalized)
    {
        return;
    }

    /*
     * Back to the value from the daemon: no change
     */
    if(!force && Reflection::has(*m_message,fields) && Reflection::read(*m_message,fields) == normalized)
    {
        Reflection::clear(*m_changes,fields);
    }
    else
    {
        Reflection::write(*m_changes,fields,normalized);
    }

    if(m_batchDepth > 0)
    {
        m_batchChanged = true;
    }
    else if(m_autoApply)
    {
        apply();
    }
    else
    {
        ++m_revision;
        emit messageChanged();
    }
}

bool DataSource::sendCommand(const QVariantMap &values)
{
    if(s_dataProvider == nullptr || m_message == nullptr)
    {
        return false;
    }

    const EnumValueDescriptor* type = legion::messages::DataType_descriptor()->FindValueByNumber(m_dataTypeValue);
    const Descriptor*          descriptor = type != nullptr ? DescriptorPool::generated_pool()->FindMessageTypeByName(type->options().GetExtension(legion::ui::command)) : nullptr;

    if(descriptor == nullptr)
    {
        setLastError(QString("Data type ").append(m_dataType).append(" has no command (legion.ui.command)"));
        return false;
    }

    std::unique_ptr<Message> command(MessageFactory::generated_factory()->GetPrototype(descriptor)->New());

    for(auto it = values.cbegin(); it != values.cend(); ++it)
    {
        const auto fields = Reflection::resolve(descriptor,it.key());

        if(fields.empty())
        {
            LOG_W(QString("DataSource ").append(m_dataType).append(": command has unknown field ").append(it.key()));
            continue;
        }

        QVariant value = it.value();

        /*
         * Enum value by name ("LOAD")
         */
        const FieldDescriptor* leaf = fields.back().field;

        if(leaf->cpp_type() == FieldDescriptor::CPPTYPE_ENUM && value.typeId() == QMetaType::QString)
        {
            const EnumValueDescriptor* enumValue = leaf->enum_type()->FindValueByName(value.toString().toStdString());

            if(enumValue == nullptr)
            {
                LOG_W(QString("DataSource ").append(m_dataType).append(": unknown enum value ").append(value.toString()));
                continue;
            }

            value = enumValue->number();
        }

        Reflection::write(*command,fields,value);
    }

    std::string data;

    if(!command->SerializeToString(&data))
    {
        setLastError("Serialize of the command failed");
        return false;
    }

    try {
        const QByteArray response = s_dataProvider->setData(static_cast<quint8>(m_dataTypeValue),QByteArray(data.data(),static_cast<qsizetype>(data.size())));
        setLastError({});
        parse(response);
    }
    catch(const std::exception& ex)
    {
        LOG_E(QString("DataSource ").append(m_dataType).append(": command failed: ").append(ex.what()));
        setLastError(ex.what());
        ++m_revision;
        emit messageChanged();
        return false;
    }

    ++m_revision;
    emit messageChanged();

    return true;
}

void DataSource::reloadAllSources()
{
    reloadAll();
}

void DataSource::beginChanges()
{
    ++m_batchDepth;
}

void DataSource::endChanges()
{
    if(m_batchDepth == 0 || --m_batchDepth > 0 || !m_batchChanged)
    {
        return;
    }

    m_batchChanged = false;

    if(m_autoApply)
    {
        apply();
    }
    else
    {
        ++m_revision;
        emit messageChanged();
    }
}

bool DataSource::modified(const QString &path) const
{
    if(m_changes == nullptr)
    {
        return false;
    }

    const auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    return !fields.empty() && Reflection::has(*m_changes,fields);
}

QVariantList DataSource::repeatedValues(const QString &path) const
{
    if(m_message == nullptr)
    {
        return {};
    }

    const auto fields = Reflection::resolve(m_message->GetDescriptor(),path);

    if(!fields.empty() && !fields.back().field->is_repeated() && fields.back().field->cpp_type() == FieldDescriptor::CPPTYPE_STRING)
    {
        QVariantList values;

        for(const auto& value : value(path).toString().split(' ',Qt::SkipEmptyParts))
        {
            values.append(value);
        }

        return values;
    }

    return Reflection::repeatedValues(*m_message,fields);
}

int DataSource::count(const QString &path) const
{
    if(m_message == nullptr)
    {
        return 0;
    }

    return Reflection::count(*m_message,Reflection::resolve(m_message->GetDescriptor(),path));
}

const Descriptor *DataSource::descriptor() const
{
    return m_message != nullptr ? m_message->GetDescriptor() : nullptr;
}

void DataSource::setLastError(const QString &error)
{
    if(error != m_lastError)
    {
        m_lastError = error;
        emit lastErrorChanged();
    }
}

}
