// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProvider.h"
#include "ProtocolProcessor.h"

namespace LenovoLegionGui {

DataProvider::DataProvider(ProtocolProcessor * protocolProcessor,QObject *parent) :
    QObject(parent),
    m_protocolProcessor(protocolProcessor)
{

}

}
