#pragma once

#include "public.h"

#include <yt/yt/server/lib/hydra_common/config.h>

#include <yt/yt/server/lib/misc/config.h>

#include <yt/yt/server/lib/io/config.h>

#include <yt/yt/library/containers/config.h>

#include <yt/yt/library/containers/disk_manager/config.h>

#include <yt/yt/ytlib/chunk_client/config.h>

#include <yt/yt/ytlib/journal_client/config.h>

#include <yt/yt/ytlib/table_client/config.h>

#include <yt/yt/client/api/config.h>

#include <yt/yt/core/concurrency/config.h>

#include <yt/yt/core/misc/arithmetic_formula.h>

#include <yt/yt/library/re2/re2.h>

#include <yt/yt/core/ytree/yson_struct.h>

namespace NYT::NDataNode {

////////////////////////////////////////////////////////////////////////////////

class TP2PConfig
    : public NYTree::TYsonStruct
{
public:
    bool Enabled;

    TSlruCacheConfigPtr BlockCache;
    TSlruCacheDynamicConfigPtr BlockCacheOverride;

    TDuration TickPeriod;
    TDuration NodeRefreshPeriod;
    TDuration RequestTimeout;
    TDuration NodeStalenessTimeout;

    TDuration IterationWaitTimeout;
    int MaxWaitingRequests;

    TDuration SessionCleaupPeriod;
    TDuration SessionTTL;

    TSlruCacheConfigPtr RequestCache;
    TSlruCacheDynamicConfigPtr RequestCacheOverride;

    TDuration ChunkCooldownTimeout;
    int MaxDistributedBytes;
    int MaxBlockSize;
    int BlockCounterResetTicks;
    int HotBlockThreshold;
    int SecondHotBlockThreshold;
    int HotBlockReplicaCount;
    int BlockRedistributionTicks;

    TBooleanFormula NodeTagFilter;

    REGISTER_YSON_STRUCT(TP2PConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TP2PConfig)

////////////////////////////////////////////////////////////////////////////////

class TChunkLocationConfig
    : public TDiskLocationConfig
{
public:
    static constexpr bool EnableHazard = true;

    //! Maximum space chunks are allowed to occupy.
    //! (If not initialized then indicates to occupy all available space on drive).
    std::optional<i64> Quota;

    // NB: actually registered as parameter by subclasses (because default value
    // is subclass-specific).
    TString MediumName;

    //! Configuration for various per-location throttlers.
    TEnumIndexedVector<EChunkLocationThrottlerKind, NConcurrency::TThroughputThrottlerConfigPtr> Throttlers;

    //! IO Engine type.
    NIO::EIOEngineType IOEngineType;

    //! IO Engine config.
    NYTree::INodePtr IOConfig;

    bool EnableUring;

    TDuration ThrottleDuration;

    //! Maximum number of bytes in the gap between two adjacent read locations
    //! in order to join them together during read coalescing.
    i64 CoalescedReadMaxGapSize;

    i64 MaxWriteRateByDwpd;

    double IOWeight;

    bool ResetUuid;

    void ApplyDynamicInplace(const TChunkLocationDynamicConfig& dynamicConfig);

    REGISTER_YSON_STRUCT(TChunkLocationConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TChunkLocationConfig)

////////////////////////////////////////////////////////////////////////////////

class TChunkLocationDynamicConfig
    : public TDiskLocationDynamicConfig
{
public:
    std::optional<NIO::EIOEngineType> IOEngineType;
    NYTree::INodePtr IOConfig;

    TEnumIndexedVector<EChunkLocationThrottlerKind, NConcurrency::TThroughputThrottlerConfigPtr> Throttlers;
    std::optional<TDuration> ThrottleDuration;

    std::optional<i64> CoalescedReadMaxGapSize;

    REGISTER_YSON_STRUCT(TChunkLocationDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TChunkLocationDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

class TStoreLocationConfig
    : public TChunkLocationConfig
{
public:
    //! A currently full location is considered to be non-full again when available space grows
    //! above this limit.
    i64 LowWatermark;

    //! A location is considered to be full when available space becomes less than #HighWatermark.
    i64 HighWatermark;

    //! All writes to the location are aborted when available space becomes less than #DisableWritesWatermark.
    i64 DisableWritesWatermark;

    //! Maximum amount of time files of a deleted chunk could rest in trash directory before
    //! being permanently removed.
    TDuration MaxTrashTtl;

    //! When free space drops below this watermark, the system starts deleting files in trash directory,
    //! starting from the eldest ones.
    i64 TrashCleanupWatermark;

    //! Period between trash cleanups.
    TDuration TrashCheckPeriod;

    //! Per-location multiplexed changelog configuration.
    NYTree::INodePtr MultiplexedChangelog;

    //! Per-location  configuration of per-chunk changelog that backs the multiplexed changelog.
    NYTree::INodePtr HighLatencySplitChangelog;

    //! Per-location configuration of per-chunk changelog that is being written directly (w/o multiplexing).
    NYTree::INodePtr LowLatencySplitChangelog;

    TStoreLocationConfigPtr ApplyDynamic(const TStoreLocationDynamicConfigPtr& dynamicConfig) const;
    void ApplyDynamicInplace(const TStoreLocationDynamicConfig& dynamicConfig);

    REGISTER_YSON_STRUCT(TStoreLocationConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TStoreLocationConfig)

////////////////////////////////////////////////////////////////////////////////

class TStoreLocationDynamicConfig
    : public TChunkLocationDynamicConfig
{
public:
    std::optional<i64> LowWatermark;
    std::optional<i64> HighWatermark;
    std::optional<i64> DisableWritesWatermark;

    std::optional<TDuration> MaxTrashTtl;
    std::optional<i64> TrashCleanupWatermark;
    std::optional<TDuration> TrashCheckPeriod;

    REGISTER_YSON_STRUCT(TStoreLocationDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TStoreLocationDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

class TCacheLocationConfig
    : public TChunkLocationConfig
{
public:
    //! Controls incoming location bandwidth used by cache.
    NConcurrency::TThroughputThrottlerConfigPtr InThrottler;

    REGISTER_YSON_STRUCT(TCacheLocationConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TCacheLocationConfig)

////////////////////////////////////////////////////////////////////////////////

class TMultiplexedChangelogConfig
    : public NHydra::TFileChangelogConfig
    , public NHydra::TFileChangelogDispatcherConfig
{
public:
    static constexpr bool EnableHazard = true;

    //! Multiplexed changelog record count limit.
    /*!
     *  When this limit is reached, the current multiplexed changelog is rotated.
     */
    int MaxRecordCount;

    //! Multiplexed changelog data size limit, in bytes.
    /*!
     *  See #MaxRecordCount.
     */
    i64 MaxDataSize;

    //! Interval between automatic changelog rotation (to avoid keeping too many non-clean records
    //! and speed up starup).
    TDuration AutoRotationPeriod;

    //! Maximum bytes of multiplexed changelog to read during
    //! a single iteration of replay.
    i64 ReplayBufferSize;

    //! Maximum number of clean multiplexed changelogs to keep.
    int MaxCleanChangelogsToKeep;

    //! Time to wait before marking a multiplexed changelog as clean.
    TDuration CleanDelay;

    //! Records bigger than BigRecordThreshold are not multiplexed.
    std::optional<i64> BigRecordThreshold;

    REGISTER_YSON_STRUCT(TMultiplexedChangelogConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TMultiplexedChangelogConfig)

////////////////////////////////////////////////////////////////////////////////

class TArtifactCacheReaderConfig
    : public virtual NChunkClient::TBlockFetcherConfig
    , public virtual NTableClient::TTableReaderConfig
    , public virtual NApi::TFileReaderConfig
{
public:
    REGISTER_YSON_STRUCT(TArtifactCacheReaderConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TArtifactCacheReaderConfig)

////////////////////////////////////////////////////////////////////////////////

class TLayerLocationConfig
    : public TDiskLocationConfig
{
public:
    //! The location is considered to be full when available space becomes less than #LowWatermark.
    i64 LowWatermark;

    //! Maximum space layers are allowed to occupy.
    //! (If not initialized then indicates to occupy all available space on drive).
    std::optional<i64> Quota;

    bool LocationIsAbsolute;

    REGISTER_YSON_STRUCT(TLayerLocationConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TLayerLocationConfig)

////////////////////////////////////////////////////////////////////////////////

class TTmpfsLayerCacheConfig
    : public NYTree::TYsonStruct
{
public:
    i64 Capacity;
    std::optional<TString> LayersDirectoryPath;
    TDuration LayersUpdatePeriod;

    REGISTER_YSON_STRUCT(TTmpfsLayerCacheConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TTmpfsLayerCacheConfig)

////////////////////////////////////////////////////////////////////////////////

class TTableSchemaCacheConfig
    : public TSlruCacheConfig
{
public:
    //! Timeout for table schema request.
    TDuration TableSchemaCacheRequestTimeout;

    REGISTER_YSON_STRUCT(TTableSchemaCacheConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TTableSchemaCacheConfig)

////////////////////////////////////////////////////////////////////////////////

class TTableSchemaCacheDynamicConfig
    : public TSlruCacheDynamicConfig
{
public:
    std::optional<TDuration> TableSchemaCacheRequestTimeout;

    REGISTER_YSON_STRUCT(TTableSchemaCacheDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TTableSchemaCacheDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

class TVolumeManagerConfig
    : public NYTree::TYsonStruct
{
public:
    NContainers::TPortoExecutorConfigPtr PortoExecutor;
    std::vector<TLayerLocationConfigPtr> LayerLocations;
    bool EnableLayersCache;
    double CacheCapacityFraction;
    int LayerImportConcurrency;

    bool EnableDiskQuota;

    bool ConvertLayersToSquashfs;

    //! Path to tar2squash binary.
    TString Tar2SquashToolPath;
    bool UseBundledTar2Squash;

    TTmpfsLayerCacheConfigPtr RegularTmpfsLayerCache;
    TTmpfsLayerCacheConfigPtr NirvanaTmpfsLayerCache;

    REGISTER_YSON_STRUCT(TVolumeManagerConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TVolumeManagerConfig)

////////////////////////////////////////////////////////////////////////////////

class TRepairReaderConfig
    : public virtual NChunkClient::TReplicationReaderConfig
    , public virtual NJournalClient::TChunkReaderConfig
{
    REGISTER_YSON_STRUCT(TRepairReaderConfig);

    static void Register(TRegistrar)
    { }
};

DEFINE_REFCOUNTED_TYPE(TRepairReaderConfig)

////////////////////////////////////////////////////////////////////////////////

// COMPAT(gritukan): Drop all the optionals in this class after configs migration.
class TMasterConnectorConfig
    : public NYTree::TYsonStruct
{
public:
    //! Period between consequent incremental data node heartbeats.
    std::optional<TDuration> IncrementalHeartbeatPeriod;

    //! Splay for data node heartbeats.
    TDuration IncrementalHeartbeatPeriodSplay;

    //! Period between consequent job heartbeats to a given cell.
    std::optional<TDuration> JobHeartbeatPeriod;

    //! Splay for job heartbeats.
    TDuration JobHeartbeatPeriodSplay;

    REGISTER_YSON_STRUCT(TMasterConnectorConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TMasterConnectorConfig)

////////////////////////////////////////////////////////////////////////////////

class TMasterConnectorDynamicConfig
    : public NYTree::TYsonStruct
{
public:
    //! Period between consequent incremental data node heartbeats.
    std::optional<TDuration> IncrementalHeartbeatPeriod;

    //! Splay for data node heartbeats.
    std::optional<TDuration> IncrementalHeartbeatPeriodSplay;

    //! Timeout for incremental data node heartbeat RPC request.
    TDuration IncrementalHeartbeatTimeout;

    //! Timeout for full data node heartbeat.
    TDuration FullHeartbeatTimeout;

    //! Period between consequent job heartbeats to a given cell.
    std::optional<TDuration> JobHeartbeatPeriod;

    //! Splay for job heartbeats.
    std::optional<TDuration> JobHeartbeatPeriodSplay;

    //! Timeout for job heartbeat RPC request.
    TDuration JobHeartbeatTimeout;

    //! Maximum number of chunk events per incremental heartbeat.
    i64 MaxChunkEventsPerIncrementalHeartbeat;

    //! Enable detailed incremental heartbeat statistics profiling.
    bool EnableProfiling;

    REGISTER_YSON_STRUCT(TMasterConnectorDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TMasterConnectorDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

class TAllyReplicaManagerDynamicConfig
    : public NYTree::TYsonStruct
{
public:
    //! Period between consequent requests to a certain node.
    TDuration AnnouncementBackoffTime;

    //! Maximum number of chunks per a single announcement request.
    i64 MaxChunksPerAnnouncementRequest;

    //! Timeout for AnnounceChunkReplicas request.
    TDuration AnnouncementRequestTimeout;

    REGISTER_YSON_STRUCT(TAllyReplicaManagerDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TAllyReplicaManagerDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

class TChunkAutotomizerConfig
    : public NYTree::TYsonStruct
{
public:
    TDuration RpcTimeout;

    // Testing options.
    bool FailJobs;
    bool SleepInJobs;

    REGISTER_YSON_STRUCT(TChunkAutotomizerConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TChunkAutotomizerConfig)

////////////////////////////////////////////////////////////////////////////////

class TDataNodeTestingOptions
    : public NYTree::TYsonStruct
{
public:
    //! This duration will be used to insert delays within [0, MaxDelay] after each
    //! chunk meta fetch for GetColumnarStatistics.
    std::optional<TDuration> ColumnarStatisticsChunkMetaFetchMaxDelay;

    bool SimulateNetworkThrottlingForGetBlockSet;

    // For testing purposes.
    bool FailReincarnationJobs;

    REGISTER_YSON_STRUCT(TDataNodeTestingOptions);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TDataNodeTestingOptions)

////////////////////////////////////////////////////////////////////////////////

class TMediumThroughputMeterConfig
    : public NIO::TGentleLoaderConfig
{
public:
    TString MediumName;
    bool Enabled;

    double VerificationInitialWindowFactor;
    double VerificationSegmentSizeFactor;
    TDuration VerificationWindowPeriod;
    double DWPDFactor;
    bool UseWorkloadModel;

    REGISTER_YSON_STRUCT(TMediumThroughputMeterConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TMediumThroughputMeterConfig)

////////////////////////////////////////////////////////////////////////////////

class TIOThroughputMeterConfig
    : public NYTree::TYsonStruct
{
public:
    bool Enabled;

    std::vector<TMediumThroughputMeterConfigPtr> Mediums;

    // Remeasure throughput after this timeout.
    TDuration TimeBetweenTests;

    // Desired estimate stage duration.
    TDuration EstimateTimeLimit;

    int MaxEstimateCongestions;

    // Max allowed overall testing duration.
    TDuration TestingTimeHardLimit;

    REGISTER_YSON_STRUCT(TIOThroughputMeterConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TIOThroughputMeterConfig)

////////////////////////////////////////////////////////////////////////////////

class TLocationHealthCheckerDynamicConfig
    : public NYTree::TYsonStruct
{
public:
    bool Enabled;

    bool EnableManualDiskFailures;

    TDuration HealthCheckPeriod;

    REGISTER_YSON_STRUCT(TLocationHealthCheckerDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TLocationHealthCheckerDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

class TChunkMergerConfig
    : public NYTree::TYsonStruct
{
public:
    // Testing options.
    bool FailShallowMergeValidation;

    REGISTER_YSON_STRUCT(TChunkMergerConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TChunkMergerConfig)

////////////////////////////////////////////////////////////////////////////////

class TChunkRepairJobDynamicConfig
    : public NYTree::TYsonStruct
{
public:
    NChunkClient::TErasureReaderConfigPtr Reader;

    i64 WindowSize;

    REGISTER_YSON_STRUCT(TChunkRepairJobDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TChunkRepairJobDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

class TJournalManagerConfig
    : public virtual NYTree::TYsonStruct
{
public:
    static constexpr bool EnableHazard = true;

    //! Configuration of multiplexed changelogs.
    TMultiplexedChangelogConfigPtr MultiplexedChangelog;

    //! Configuration of per-chunk changelogs that back the multiplexed changelog.
    NHydra::TFileChangelogConfigPtr HighLatencySplitChangelog;

    //! Configuration of per-chunk changelogs that are being written directly (w/o multiplexing).
    NHydra::TFileChangelogConfigPtr LowLatencySplitChangelog;

    REGISTER_YSON_STRUCT(TJournalManagerConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TJournalManagerConfig)

////////////////////////////////////////////////////////////////////////////////

class TDataNodeConfig
    : public TJournalManagerConfig
{
public:
    //! Timeout for lease transactions.
    TDuration LeaseTransactionTimeout;

    //! Period between consequent lease transaction pings.
    TDuration LeaseTransactionPingPeriod;

    //! Period between consequent incremental heartbeats.
    TDuration IncrementalHeartbeatPeriod;

    //! Splay for incremental heartbeats.
    TDuration IncrementalHeartbeatPeriodSplay;

    //! Controls incremental heartbeats from node to master.
    NConcurrency::TThroughputThrottlerConfigPtr IncrementalHeartbeatThrottler;

    //! Period between consequent registration attempts.
    TDuration RegisterRetryPeriod;

    //! Splay for consequent registration attempts.
    TDuration RegisterRetrySplay;

    //! Timeout for RegisterNode requests.
    TDuration RegisterTimeout;

    //! Timeout for NodeTrackerService:IncrementalHeartbeat requests.
    TDuration IncrementalHeartbeatTimeout;

    //! Timeout for NodeTrackerService:FullHeartbeat requests.
    TDuration FullHeartbeatTimeout;

    //! Timeout for JobTrackerService:Heartbeat requests.
    TDuration JobHeartbeatTimeout;

    //! Cache for chunk metas.
    TSlruCacheConfigPtr ChunkMetaCache;

    //! Cache for blocks extensions.
    TSlruCacheConfigPtr BlocksExtCache;

    //! Cache for partition block metas.
    TSlruCacheConfigPtr BlockMetaCache;

    //! Cache for all types of blocks.
    NChunkClient::TBlockCacheConfigPtr BlockCache;

    //! Opened blob chunks cache.
    TSlruCacheConfigPtr BlobReaderCache;

    //! Opened changelogs cache.
    TSlruCacheConfigPtr ChangelogReaderCache;

    //! Table schema and row key comparer cache.
    TTableSchemaCacheConfigPtr TableSchemaCache;

    //! Configuration of the interaction with the host disk manager.
    NContainers::TDiskManagerProxyConfigPtr DiskManagerProxy;

    //! Upload session timeout.
    /*!
     * Some activity must be happening in a session regularly (i.e. new
     * blocks uploaded or sent to other data nodes). Otherwise
     * the session expires.
     */
    TDuration SessionTimeout;

    TDuration SessionBlockReorderTimeout;

    //! Timeout for "PutBlocks" requests to other data nodes.
    TDuration NodeRpcTimeout;

    //! Period between peer updates (see TBlockPeerUpdater).
    TDuration PeerUpdatePeriod;

    //! Peer update expiration time (see TBlockPeerUpdater).
    TDuration PeerUpdateExpirationTime;

    //! Read requests are throttled when the number of bytes queued at Bus layer exceeds this limit.
    //! This is a global limit.
    //! Cf. TTcpDispatcherStatistics::PendingOutBytes
    i64 NetOutThrottlingLimit;

    //! Smoothing interval for net out limit throttling.
    TDuration NetOutThrottlingDuration;

    //! Write requests are throttled when the number of bytes queued for write exceeds this limit.
    //! This is a per-location limit.
    i64 DiskWriteThrottlingLimit;

    //! Read requests are throttled when the number of bytes scheduled for read exceeds this limit.
    //! This is a per-location limit.
    i64 DiskReadThrottlingLimit;

    //! Regular storage locations.
    std::vector<TStoreLocationConfigPtr> StoreLocations;

    //! Cached chunks location.
    std::vector<TCacheLocationConfigPtr> CacheLocations;

    //! Manages layers and root volumes for Porto job environment.
    TVolumeManagerConfigPtr VolumeManager;

    //! Writer configuration used to replicate chunks.
    NChunkClient::TReplicationWriterConfigPtr ReplicationWriter;

    //! Reader configuration used to repair chunks (both blob and journal).
    TRepairReaderConfigPtr RepairReader;

    //! Writer configuration used to repair chunks.
    NChunkClient::TReplicationWriterConfigPtr RepairWriter;

    //! Reader configuration used to seal chunks.
    NJournalClient::TChunkReaderConfigPtr SealReader;

    //! Reader configuration used to merge chunks.
    NChunkClient::TReplicationReaderConfigPtr MergeReader;

    //! Writer configuration used to merge chunks.
    NChunkClient::TMultiChunkWriterConfigPtr MergeWriter;

    //! Reader configuration used to autotomize chunks.
    NJournalClient::TChunkReaderConfigPtr AutotomyReader;

    //! Writer configuration used to autotomize chunks.
    NChunkClient::TReplicationWriterConfigPtr AutotomyWriter;

    //! Writer configuration used to reincarnate chunks.
    NChunkClient::TMultiChunkWriterConfigPtr ReincarnationWriter;

    //! Configuration for various Data Node throttlers. Used when fair throttler is not enabled.
    TEnumIndexedVector<EDataNodeThrottlerKind, NConcurrency::TRelativeThroughputThrottlerConfigPtr> Throttlers;

    //! Configuration for RPS out throttler.
    NConcurrency::TThroughputThrottlerConfigPtr ReadRpsOutThrottler;

    //! Configuration for RPS throttler of ally replica manager.
    NConcurrency::TThroughputThrottlerConfigPtr AnnounceChunkReplicaRpsOutThrottler;

    //! Runs periodic checks against disks.
    TDiskHealthCheckerConfigPtr DiskHealthChecker;

    //! Publish disabled locations to master.
    bool PublishDisabledLocations;

    //! Maximum number of concurrent balancing write sessions.
    int MaxWriteSessions;

    //! Maximum number of blocks to fetch via a single range request.
    int MaxBlocksPerRead;

    //! Maximum number of bytes to fetch via a single range request.
    i64 MaxBytesPerRead;

    //! Desired number of bytes per disk write in a blob chunks.
    i64 BytesPerWrite;

    //! Enables block checksums validation.
    bool ValidateBlockChecksums;

    //! The time after which any registered placement info expires.
    TDuration PlacementExpirationTime;

    //! Controls if cluster and cell directories are to be synchronized on connect.
    //! Useful for tests.
    bool SyncDirectoriesOnConnect;

    //! The number of threads in StorageHeavy thread pool (used for extracting chunk meta, handling
    //! chunk slices, columnar statistic etc).
    int StorageHeavyThreadCount;

    //! The number of threads in StorageLight thread pool (used for reading chunk blocks).
    int StorageLightThreadCount;

    //! Number of threads in DataNodeLookup thread pool (used for row lookups).
    int StorageLookupThreadCount;

    //! Number of replication errors sent in heartbeat.
    int MaxReplicationErrorsInHeartbeat;

    //! Number of tablet errors sent in heartbeat.
    int MaxTabletErrorsInHeartbeat;

    //! Fraction of GetBlockSet/GetBlockRange RPC timeout, after which reading routine tries
    //! to return all blocks read up to moment (in case at least one block is read; otherwise
    //! it still tries to read at least one block).
    double BlockReadTimeoutFraction;

    //! Fraction of the GetColumnarStatistics RPC timeout, after which early exit is performed and currently uncompleted
    //! chunk fetches are failed with a timeout error.
    //! The enable_early_exit field has to be set to true in the request options for this option to have any effect.
    double ColumnarStatisticsReadTimeoutFraction;

    //! Delay between node initializatin and start of background artifact validation.
    TDuration BackgroundArtifactValidationDelay;

    //! Master connector config.
    TMasterConnectorConfigPtr MasterConnector;

    //! Config for the new P2P implementation.
    TP2PConfigPtr P2P;

    //! Testing options.
    TDataNodeTestingOptionsPtr TestingOptions;

    i64 GetCacheCapacity() const;

    REGISTER_YSON_STRUCT(TDataNodeConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TDataNodeConfig)

////////////////////////////////////////////////////////////////////////////////

class TDataNodeDynamicConfig
    : public NYTree::TYsonStruct
{
public:
    std::optional<int> StorageHeavyThreadCount;
    std::optional<int> StorageLightThreadCount;
    std::optional<int> StorageLookupThreadCount;

    //! Number of threads in MasterJob thread pool (used for master jobs execution).
    int MasterJobThreadCount;

    TEnumIndexedVector<EDataNodeThrottlerKind, NConcurrency::TRelativeThroughputThrottlerConfigPtr> Throttlers;
    NConcurrency::TThroughputThrottlerConfigPtr ReadRpsOutThrottler;
    NConcurrency::TThroughputThrottlerConfigPtr AnnounceChunkReplicaRpsOutThrottler;

    TSlruCacheDynamicConfigPtr ChunkMetaCache;
    TSlruCacheDynamicConfigPtr BlocksExtCache;
    TSlruCacheDynamicConfigPtr BlockMetaCache;
    NChunkClient::TBlockCacheDynamicConfigPtr BlockCache;
    TSlruCacheDynamicConfigPtr BlobReaderCache;
    TSlruCacheDynamicConfigPtr ChangelogReaderCache;
    TTableSchemaCacheDynamicConfigPtr TableSchemaCache;

    TMasterConnectorDynamicConfigPtr MasterConnector;
    TAllyReplicaManagerDynamicConfigPtr AllyReplicaManager;

    //! Configuration of the interaction with the host disk manager.
    NContainers::TDiskManagerProxyDynamicConfigPtr DiskManagerProxy;

    //! Prepared chunk readers are kept open during this period of time after the last use.
    TDuration ChunkReaderRetentionTimeout;

    //! Reader configuration used to download chunks into cache.
    TArtifactCacheReaderConfigPtr ArtifactCacheReader;

    //! If |true|, node will abort when location becomes disabled.
    bool AbortOnLocationDisabled;

    //! Publish disabled locations to master.
    std::optional<bool> PublishDisabledLocations;

    NContainers::TActiveDiskCheckerDynamicConfigPtr ActiveDiskChecker;

    TP2PConfigPtr P2P;

    TChunkAutotomizerConfigPtr ChunkAutotomizer;

    TDuration IOStatisticsUpdateTimeout;

    // COMPAT(gritukan, capone212)
    NChunkClient::TErasureReaderConfigPtr AdaptiveChunkRepairJob;

    TIOThroughputMeterConfigPtr IOThroughputMeter;

    TChunkMergerConfigPtr ChunkMerger;

    TChunkRepairJobDynamicConfigPtr ChunkRepairJob;

    TLocationHealthCheckerDynamicConfigPtr LocationHealthChecker;

    THashMap<TString, TStoreLocationDynamicConfigPtr> StoreLocationConfigPerMedium;

    std::optional<i64> NetOutThrottlingLimit;

    std::optional<i64> DiskWriteThrottlingLimit;
    std::optional<i64> DiskReadThrottlingLimit;

    //! If the total pending read size exceeds the limit, all writes to this location become disabled.
    std::optional<i64> DisableLocationWritesPendingReadSizeHighLimit;

    //! If writes to the location were earlier disabled due to #DisableLocationWritesPendingReadSizeHighLimit,
    //! writes are re-enabled once the total pending read size drops below this limit.
    std::optional<i64> DisableLocationWritesPendingReadSizeLowLimit;

    //! Testing options.
    TDataNodeTestingOptionsPtr TestingOptions;

    REGISTER_YSON_STRUCT(TDataNodeDynamicConfig);

    static void Register(TRegistrar registrar);
};

DEFINE_REFCOUNTED_TYPE(TDataNodeDynamicConfig)

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NDataNode
