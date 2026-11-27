// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverIGpuFrequency.h"
#include "../KitLog.h"

#include <algorithm>
#include <vector>

namespace LenovoLegionDaemon::Kit {

SysFsDriverIGpuFrequency::SysFsDriverIGpuFrequency() : SysFsDriver(DRIVER_NAME,"/sys/class/drm",{}) {}

void SysFsDriverIGpuFrequency::init()
{
    LOG_D(__PRETTY_FUNCTION__);

    clean();

    std::error_code                    error;
    std::vector<std::filesystem::path> cards;

    for(const auto& entry : std::filesystem::directory_iterator(m_path,error))
    {
        const std::string name = entry.path().filename().string();

        if(name.starts_with("card") && name.find('-') == std::string::npos)
        {
            cards.push_back(entry.path());
        }
    }

    std::sort(cards.begin(),cards.end());

    for(const auto& card : cards)
    {
        /*
         * i915: the files of the card
         */
        if(std::filesystem::exists(card / "gt_max_freq_mhz") && std::filesystem::exists(card / "gt_RP0_freq_mhz"))
        {
            m_descriptor["i915"]      = card;
            m_descriptor["min"]       = card / "gt_min_freq_mhz";
            m_descriptor["max"]       = card / "gt_max_freq_mhz";
            m_descriptor["rpn"]       = card / "gt_RPn_freq_mhz";
            m_descriptor["rp0"]       = card / "gt_RP0_freq_mhz";

            if(std::filesystem::exists(card / "gt_boost_freq_mhz")) m_descriptor["boost"]     = card / "gt_boost_freq_mhz";
            if(std::filesystem::exists(card / "gt_RP1_freq_mhz"))   m_descriptor["efficient"] = card / "gt_RP1_freq_mhz";
            if(std::filesystem::exists(card / "gt_act_freq_mhz"))   m_descriptor["current"]   = card / "gt_act_freq_mhz";

            LOG_D(QString("Intel iGPU frequency (i915) found: ") + card.c_str());
            return;
        }

        /*
         * xe: the first GT of the first tile
         */
        const std::filesystem::path freq = card / "device" / "tile0" / "gt0" / "freq0";

        if(std::filesystem::exists(freq / "max_freq") && std::filesystem::exists(freq / "rp0_freq"))
        {
            m_descriptor["xe"]        = card;
            m_descriptor["min"]       = freq / "min_freq";
            m_descriptor["max"]       = freq / "max_freq";
            m_descriptor["rpn"]       = freq / "rpn_freq";
            m_descriptor["rp0"]       = freq / "rp0_freq";

            if(std::filesystem::exists(freq / "rpe_freq")) m_descriptor["efficient"] = freq / "rpe_freq";
            if(std::filesystem::exists(freq / "act_freq")) m_descriptor["current"]   = freq / "act_freq";

            LOG_D(QString("Intel iGPU frequency (xe) found: ") + freq.c_str());
            return;
        }
    }

    LOG_D("Intel iGPU frequency not found");
}

}
