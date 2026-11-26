// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include <MessageRegistry.pb.h>
#include <RGBController.pb.h>

#include <string>

namespace LenovoLegionDaemon::Kit {

/*
 * 4-zone RGB keyboard of the Legion notebooks (e.g. Legion 5 / Slim 5 Gen 6 - 8): ITE 048d:c9xx, HID collection
 * 0xFF89:0xCC with the feature report 0xCC of 33 bytes, the same protocol as Lenovo Legion Toolkit
 * (LENOVO_RGB_KEYBOARD_STATE). The keyboard can not be read, the state is the one set last (the settings restore it).
 * Before a write the lights are taken from the keyboard firmware (game zone SetLightControlOwner 1), when the Lenovo
 * Legion kernel driver provides it.
 */
class DataProviderRGB4Zone : public SysFsDataProvider
{
public:

    explicit DataProviderRGB4Zone(SysFsDriverManager* sysFsDriverManager);

    QByteArray serializeAndGetData()                         override;
    QByteArray deserializeAndSetData(const QByteArray &data) override;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

    void init()  override;
    void clean() override;

    static constexpr quint8  dataType = legion::messages::DataType::KEYBOARD_FOUR_ZONE;

private:

    void settable(legion::messages::Keyboard4Zone& keyboard,Origin origin) const;

    /*
     * Writes the state to the keyboard, false on an error (logged)
     */
    bool write(const legion::messages::Keyboard4Zone& state);

    /*
     * Path of the HID device of the 4-zone keyboard, empty without one
     */
    static std::string find();

private:

    std::string                        m_path;
    legion::messages::Keyboard4Zone    m_state;
};

}
