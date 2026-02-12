// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include "DataProvider.h"

#include <vector>
#include <memory>


struct hid_device_info;

namespace LenovoLegionDaemon {

#define HID_INTERFACE_ANY -1

class RGBController;
class DataProviderRGBController : public DataProvider
{
    Q_OBJECT

public:

    typedef std::function<RGBController* (const hid_device_info &, const std::string &)> HIDDeviceDetectorFunction;


private:

    struct HIDDeviceDetectorBlock
    {
        const std::string m_name;
        const uint16_t    m_vid;
        const uint16_t    m_pid;
        const uint16_t    m_pidMask;

        const HIDDeviceDetectorFunction m_function;

        bool compare(const hid_device_info& info);
    };


public:

    enum ERROR_CODES : int {
        INVALID_DATA                        = -1,
        SERIALIZE_ERROR                     = -2
    };

public:

    DataProviderRGBController(QObject* parent);

    virtual ~DataProviderRGBController() override;

    virtual QByteArray serializeAndGetData(const QByteArray&)                                const override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)                                    override;

    virtual void init() override;
    virtual void clean() override;


    virtual void kernelEventHandler(const LenovoLegionDaemon::SysFsDriver::SubsystemEvent &)       override;
public:

    static void registerControler(std::string               name,
                                  HIDDeviceDetectorFunction det,
                                  uint16_t                  vid,
                                  uint16_t                  pid,
                                  uint16_t                  pidMask);

private:

    static std::vector<HIDDeviceDetectorBlock>  hidDeviceDetectorsBlocks;;

private:

    std::unique_ptr<RGBController> m_rgbController;

public:

    static constexpr quint8  dataType = 19;
};

}
