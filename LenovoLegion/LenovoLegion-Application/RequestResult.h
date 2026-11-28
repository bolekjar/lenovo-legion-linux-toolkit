// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QByteArray>

namespace LenovoLegionGui {

/*
 * Result of a request: the response, the error description from the daemon (ERROR_RESPONSE, empty for other failures:
 * timeout, invalid message, ...) or the reason why the data are not available on this machine (NOT_AVAILABLE_RESPONSE)
 */
struct RequestResult {
    enum Status { OK, ERROR, NOT_AVAILABLE };

    Status     status = ERROR;
    QByteArray data;
};

}
