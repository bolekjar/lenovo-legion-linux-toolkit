#include "SysFsDataProvider.h"


#include <QFile>
#include <QTextStream>

namespace LenovoLegionDaemon {

SysFsDataProvider::SysFsDataProvider(SysFsDriverManager* sysFsDriverManager, QObject* parent, quint8 dataType) :
    DataProvider(parent,dataType),
    m_sysFsDriverManager(sysFsDriverManager) {}


QString SysFsDataProvider::getData(const std::filesystem::path &path)
{
    QFile file(path);

    if(!file.open(QIODeviceBase::ReadOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_READING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=ReadOnly !").c_str());
    }

    return QTextStream(&file).readAll().trimmed();
}
void SysFsDataProvider::setData(const std::filesystem::path &path, quint8 value)
{
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;
}

void SysFsDataProvider::setData(const std::filesystem::path &path, quint16 value)
{
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;
}

void SysFsDataProvider::setData(const std::filesystem::path &path, quint32 value)
{
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;
}

void SysFsDataProvider::setData(const std::filesystem::path &path, quint64 value)
{
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;
}

void SysFsDataProvider::setData(const std::filesystem::path &path, bool value)
{
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << (quint8)(value ? 1 : 0);
}

void SysFsDataProvider::setData(const std::filesystem::path &path, const std::vector<quint8>& values)
{
    QFile file(path);
    QTextStream   out(&file);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    for (size_t index = 0; index < values.size(); ++index) {

        if (index == values.size() - 1)
        {
            out << values[index];
        }
        else
        {
            out << values[index] << ",";
        }
    }
}

void SysFsDataProvider::setData(const std::filesystem::path &path, const std::vector<quint32> &values)
{
    QFile file(path);
    QTextStream   out(&file);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    for (size_t index = 0; index < values.size(); ++index) {

        if (index == values.size() - 1)
        {
            out << values[index];
        }
        else
        {
            out << values[index] << ",";
        }
    }
}

void SysFsDataProvider::setData(const std::filesystem::path &path, const std::string_view &value)
{
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.c_str()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value.data();
}


void SysFsDataProvider::setData(const std::filesystem::path &path, qint8 value) {
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;
}


void SysFsDataProvider::setData(const std::filesystem::path &path, qint16 value){
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;
}


void SysFsDataProvider::setData(const std::filesystem::path &path, qint32 value){
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;
}


void SysFsDataProvider::setData(const std::filesystem::path &path, qint64 value){
    QFile file(path);

    if(!file.open(QIODeviceBase::WriteOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !").c_str());
    }

    QTextStream(&file) << value;

}





}
