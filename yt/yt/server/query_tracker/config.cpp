#include "config.h"

#include <yt/yt/ytlib/api/native/config.h>

#include <yt/yt/client/security_client/public.h>

namespace NYT::NQueryTracker {

using namespace NSecurityClient;

////////////////////////////////////////////////////////////////////////////////

void TEngineConfigBase::Register(TRegistrar registrar)
{
    registrar.Parameter("query_state_write_backoff", &TThis::QueryStateWriteBackoff)
        .Default(TDuration::Seconds(1));
    registrar.Parameter("row_count_limit", &TThis::RowCountLimit)
        .Default(10'000);
}

////////////////////////////////////////////////////////////////////////////////

void TYqlEngineConfig::Register(TRegistrar /*registrar*/)
{ }

////////////////////////////////////////////////////////////////////////////////

void TQueryTrackerDynamicConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("active_query_acquisition_period", &TThis::ActiveQueryAcquisitionPeriod)
        .Default(TDuration::Seconds(1));
    registrar.Parameter("active_query_lease_timeout", &TThis::ActiveQueryLeaseTimeout)
        .Default(TDuration::Seconds(5));
    registrar.Parameter("active_query_ping_period", &TThis::ActiveQueryPingPeriod)
        .Default(TDuration::Seconds(1));
    registrar.Parameter("query_finish_backoff", &TThis::QueryFinishBackoff)
        .Default(TDuration::Seconds(1));
    registrar.Parameter("ql_engine", &TThis::QlEngine)
        .DefaultNew();
    registrar.Parameter("yql_engine", &TThis::YqlEngine)
        .DefaultNew();
    registrar.Parameter("mock_engine", &TThis::MockEngine)
        .DefaultNew();
}

////////////////////////////////////////////////////////////////////////////////

void TQueryTrackerServerConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("abort_on_unrecognized_options", &TThis::AbortOnUnrecognizedOptions)
        .Default(false);
    registrar.Parameter("user", &TThis::User)
        .Default(QueryTrackerUserName);
    registrar.Parameter("cypress_annotations", &TThis::CypressAnnotations)
        .Default(NYTree::BuildYsonNodeFluently()
            .BeginMap()
            .EndMap()
        ->AsMap());
    registrar.Parameter("election_manager", &TThis::ElectionManager)
        .DefaultNew();
    registrar.Parameter("dynamic_config_manager", &TThis::DynamicConfigManager)
        .DefaultNew();
    registrar.Parameter("dynamic_config_path", &TThis::DynamicConfigPath)
        .Default();
    registrar.Parameter("root", &TThis::Root)
        .Default();
    registrar.Parameter("create_state_tables_on_startup", &TThis::CreateStateTablesOnStartup)
        .Default(false);

    registrar.Postprocessor([] (TThis* config) {
        if (auto& lockPath = config->ElectionManager->LockPath; lockPath.empty()) {
            lockPath = config->Root + "/leader_lock";
        }
        if (auto& dynamicConfigPath = config->DynamicConfigPath; dynamicConfigPath.empty()) {
            dynamicConfigPath = config->Root + "/config";
        }
    });
};

////////////////////////////////////////////////////////////////////////////////

void TQueryTrackerServerDynamicConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("query_tracker", &TThis::QueryTracker)
        .DefaultNew();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NQueryTracker