// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriver.h"


namespace LenovoLegionDaemon {

SysFsDriver::SysFsDriver(const QString& name,const std::filesystem::path& path,const KernelEvent::Filter& filter,QObject *parent,QString module) : QObject (parent), m_name(name),m_path(path),m_filter(filter),m_module(module.isEmpty() ? name : module) {}

void SysFsDriver::clean()
{
    m_descriptor.clear();
    m_descriptorsInVector.clear();
}

bool SysFsDriver::isLoaded() const
{
    return !m_descriptor.empty() || !m_descriptorsInVector.empty();
}

void SysFsDriver::validate() const
{
    for (const auto& path : m_descriptor) {
        if(!std::filesystem::exists(path))
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::VALIDATION_ERROR,std::string("Driver not found in path: ").append(path).c_str());
        }
    }

    for (const auto& descriptor : m_descriptorsInVector) {
        for (const auto& path : descriptor) {            
            if(!std::filesystem::exists(path))
            {
                THROW_EXCEPTION(exception_T,ERROR_CODES::VALIDATION_ERROR,std::string("Driver not found in path: ").append(path).c_str());
            }
        }
    }
}

void SysFsDriver::handleKernelEvent(const KernelEvent::Event &)
{}

void SysFsDriver::blockKernelEvent(bool block)
{
    m_blockKernelEvent = block;
}

const SysFsDriver::DescriptorsInVectorType &SysFsDriver::descriptorsInVector() const
{
    if(m_descriptorsInVector.empty())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_AVAILABLE,"Driver " + m_name.toStdString() + " is not loaded !");
    }

    for (const auto& descriptor : m_descriptorsInVector) {

        if(descriptor.isEmpty())
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_AVAILABLE,"Driver " + m_name.toStdString() + " is not loaded !");
        }
    }

    return m_descriptorsInVector;
}

const SysFsDriver::DescriptorType &SysFsDriver::desriptor() const
{
    if(m_descriptor.empty())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_AVAILABLE,"Driver " + m_name.toStdString() + " is not loaded !");
    }

    return m_descriptor;
}

}
