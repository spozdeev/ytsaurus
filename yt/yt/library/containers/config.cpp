#include "config.h"

namespace NYT::NContainers {

////////////////////////////////////////////////////////////////////////////////

bool TCGroupConfig::IsCGroupSupported(const TString& cgroupType) const
{
    auto it = std::find_if(
        SupportedCGroups.begin(),
        SupportedCGroups.end(),
        [&] (const TString& type) {
            return type == cgroupType;
        });
    return it != SupportedCGroups.end();
}

void TCGroupConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("supported_cgroups", &TThis::SupportedCGroups)
        .Default();

    registrar.Postprocessor([] (TThis* config) {
        for (const auto& type : config->SupportedCGroups) {
            if (!IsValidCGroupType(type)) {
                THROW_ERROR_EXCEPTION("Invalid cgroup type %Qv", type);
            }
        }
    });
}

////////////////////////////////////////////////////////////////////////////////

void TPortoExecutorConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("retries_timeout", &TThis::RetriesTimeout)
        .Default(TDuration::Seconds(10));
    registrar.Parameter("poll_period", &TThis::PollPeriod)
        .Default(TDuration::MilliSeconds(100));
    registrar.Parameter("api_timeout", &TThis::ApiTimeout)
        .Default(TDuration::Minutes(5));
    registrar.Parameter("api_disk_timeout", &TThis::ApiDiskTimeout)
        .Default(TDuration::Minutes(30));
    registrar.Parameter("enable_network_isolation", &TThis::EnableNetworkIsolation)
        .Default(true);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NContainers