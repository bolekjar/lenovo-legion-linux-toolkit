#include "Utils.h"


namespace LenovoLegionGui {


    namespace Utils
    {
        namespace ProtoBuf {

            void forAllCpuTopologyRange(const std::function<bool (const int)> &func, const ::google::protobuf::RepeatedPtrField<legion::messages::CPUTopology::ActiveCPUsRange> &cpuRange)
            {
                for (const auto& activeCpuRange: cpuRange) {
                    for(uint32_t i = activeCpuRange.cpu_min(); i < activeCpuRange.cpu_max() + 1; ++i)
                    {
                        if(!func(i)) {
                            return;
                        }
                    }
                }
            }

            void forAllCpuTopologyRange(const std::function<bool (const int)> &func, const legion::messages::CPUTopology::ActiveCPUsRange &cpuRange){
                for(uint32_t i = cpuRange.cpu_min(); i < cpuRange.cpu_max() + 1; ++i)
                {
                    if(!func(i)) {
                        return;
                    }
                }
            }


            void forAllCpu(const std::function<bool (const legion::messages::CPUFrequency::CPUX&,const size_t)> &func, const ::google::protobuf::RepeatedPtrField<legion::messages::CPUFrequency::CPUX> &cpus)
            {
                for(int i = 0; i < cpus.size(); ++i)
                {
                    if(!func(cpus.at(i),i)) {
                        return;
                    }
                }
            }
        }
    }



    namespace Utils
    {
        namespace Layout {

            void deleteLayoutItem(QLayoutItem *layout)
            {
                if(layout != nullptr)
                {
                    if(layout->widget() != nullptr)
                    {
                        layout->widget()->deleteLater();
                    }

                    if(layout->layout() != nullptr)
                    {
                        layout->layout()->deleteLater();
                    }
                    delete layout;
                }
            }


            void removeAllLayoutWidgets(QLayout &layout)
            {
                while (layout.count() > 0)
                {
                    deleteLayoutItem(layout.takeAt(0));
                }
            }

            void forAllLayoutsDo(QLayout &layout, const std::function<void (QLayoutItem &)> &func)
            {
                for(int i = 0; i < layout.count(); ++i)
                {
                    if(layout.itemAt(i) != nullptr)
                    {
                        func(*layout.itemAt(i));
                    }
                }
            }

        }
    }
}
