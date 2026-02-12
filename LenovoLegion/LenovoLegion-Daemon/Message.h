// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QtTypes>
#include <QString>

namespace LenovoLegionDaemon {

struct __attribute__ ((__packed__)) MessageHeader {

    enum Type : quint8 {

        //GUI ----> Daemon, has reponse
        GET_DATA_REQUEST   = 0,
        SET_DATA_REQUEST   = 1,

        //Daemon ----> GUI
        GET_DATA_RESPONSE  = 2,
        SET_DATA_RESPONSE  = 3,

        //Daemon ----> GUI, no reponse
        NOTIFICATION       = 4
    };


    /*
     * Header
     */
    Type        m_type;
    quint8      m_dataType;
    qsizetype   m_dataLength;
};


}
