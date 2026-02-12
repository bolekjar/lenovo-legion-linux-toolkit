#include "DataProvider.h"



namespace LenovoLegionDaemon {

DataProvider::DataProvider(QObject* parent, quint8 dataType) : QObject(parent),m_dataType(dataType) {}

}
