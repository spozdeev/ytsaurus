#include "config.h"

namespace NYT::NNodeTrackerServer {

////////////////////////////////////////////////////////////////////////////////

void TNodeDiscoveryManagerConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("update_period", &TThis::UpdatePeriod)
        .Default(TDuration::Seconds(30));
    registrar.Parameter("peer_count", &TThis::PeerCount)
        .GreaterThanOrEqual(0)
        .Default(10);
    registrar.Parameter("max_peers_per_rack", &TThis::MaxPeersPerRack)
        .GreaterThan(0)
        .Default(1);
    registrar.Parameter("node_tag_filter", &TThis::NodeTagFilter)
        .Default();
}

////////////////////////////////////////////////////////////////////////////////

void TNodeGroupConfigBase::Register(TRegistrar registrar)
{
    registrar.Parameter("max_concurrent_node_registrations", &TThis::MaxConcurrentNodeRegistrations)
        .Default(20)
        .GreaterThanOrEqual(0);
}

////////////////////////////////////////////////////////////////////////////////

void TNodeGroupConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("node_tag_filter", &TThis::NodeTagFilter);
}

////////////////////////////////////////////////////////////////////////////////

void TDynamicNodeTrackerConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("node_groups", &TThis::NodeGroups)
        .Default();

    registrar.Parameter("total_node_statistics_update_period", &TThis::TotalNodeStatisticsUpdatePeriod)
        .Default(TDuration::Seconds(60));

    registrar.Parameter("incremental_node_states_gossip_period", &TThis::IncrementalNodeStatesGossipPeriod)
        .Default(TDuration::Seconds(1));
    registrar.Parameter("full_node_states_gossip_period", &TThis::FullNodeStatesGossipPeriod)
        .Default(TDuration::Minutes(1));

    registrar.Parameter("max_concurrent_node_registrations", &TThis::MaxConcurrentNodeRegistrations)
        .Default(20)
        .GreaterThanOrEqual(0);
    registrar.Parameter("max_concurrent_node_unregistrations", &TThis::MaxConcurrentNodeUnregistrations)
        .Default(20)
        .GreaterThan(0);

    registrar.Parameter("max_concurrent_cluster_node_heartbeats", &TThis::MaxConcurrentClusterNodeHeartbeats)
        .Default(50)
        .GreaterThan(0);
    registrar.Parameter("max_concurrent_exec_node_heartbeats", &TThis::MaxConcurrentExecNodeHeartbeats)
        .Default(50)
        .GreaterThan(0);

    registrar.Parameter("max_concurrent_full_heartbeats", &TThis::MaxConcurrentFullHeartbeats)
        .Default(1)
        .GreaterThan(0);
    registrar.Parameter("max_concurrent_incremental_heartbeats", &TThis::MaxConcurrentIncrementalHeartbeats)
        .Default(10)
        .GreaterThan(0);

    registrar.Parameter("force_node_heartbeat_request_timeout", &TThis::ForceNodeHeartbeatRequestTimeout)
        .Default(TDuration::Seconds(1));

    registrar.Parameter("master_cache_manager", &TThis::MasterCacheManager)
        .DefaultNew();
    registrar.Parameter("timestamp_provider_manager", &TThis::TimestampProviderManager)
        .DefaultNew();

    registrar.Parameter("use_new_heartbeats", &TThis::UseNewHeartbeats)
        .Default(false);

    registrar.Parameter("preserve_rack_for_new_host", &TThis::PreserveRackForNewHost)
        .Default(false)
        .DontSerializeDefault();

    registrar.Parameter("enable_structured_log", &TThis::EnableStructuredLog)
        .Default(false)
        .DontSerializeDefault();

    registrar.Parameter("enable_node_cpu_statistics", &TThis::EnableNodeCpuStatistics)
        .Default(false)
        .DontSerializeDefault();

    registrar.Parameter("profiling_period", &TThis::ProfilingPeriod)
        .Default(DefaultProfilingPeriod);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NNodeTrackerServer