#include "replication_card_channel_factory.h"

#include "chaos_cell_directory_synchronizer.h"
#include "config.h"
#include "private.h"
#include "replication_card_residency_cache.h"

#include <yt/yt/ytlib/election/alien_cell_peer_channel_factory.h>

#include <yt/yt/ytlib/hive/cell_directory.h>

#include <yt/yt/client/chaos_client/helpers.h>

#include <yt/yt/core/misc/public.h>
#include <yt/yt/core/misc/atomic_object.h>

#include <yt/yt/core/rpc/helpers.h>
#include <yt/yt/core/rpc/public.h>
#include <yt/yt/core/rpc/roaming_channel.h>

#include <yt/yt/core/rpc/bus/channel.h>

#include <yt/yt/core/ytree/fluent.h>

#include <yt/yt/core/bus/tcp/config.h>

namespace NYT::NChaosClient {

using namespace NRpc;
using namespace NHiveClient;
using namespace NYTree;
using namespace NHydra;
using namespace NObjectClient;

////////////////////////////////////////////////////////////////////////////////

class TReplicationCardChannelProvider
    : public IRoamingChannelProvider
{
public:
    TReplicationCardChannelProvider(
        TReplicationCardId replicationCardId,
        ICellDirectoryPtr cellDirectory,
        IReplicationCardResidencyCachePtr residencyCache,
        IChaosCellDirectorySynchronizerPtr synchronizer,
        EPeerKind peerKind,
        TReplicationCardChannelConfigPtr config)
        : Config_(std::move(config))
        , CellDirectory_(std::move(cellDirectory))
        , ReplicationCardResidencyCache_(std::move(residencyCache))
        , PeerKind_(peerKind)
        , ReplicationCardId_(replicationCardId)
        , EndpointDescription_(Format("ReplicationCardId:%v", replicationCardId))
        , EndpointAttributes_(ConvertToAttributes(BuildYsonStringFluently()
            .BeginMap()
                .Item("replication_card_id").Value(replicationCardId)
            .EndMap()))
        , UnavailableError_(TError(NRpc::EErrorCode::Unavailable, "Replication card is not available")
            << TErrorAttribute("endpoint", EndpointDescription_))
        , Logger(ChaosClientLogger
            .WithTag("ReplicationCardId: %v", replicationCardId))
    {
        synchronizer->Sync();
    }

    const TString& GetEndpointDescription() const override
    {
        return EndpointDescription_;
    }

    const IAttributeDictionary& GetEndpointAttributes() const override
    {
        return *EndpointAttributes_;
    }

    TFuture<IChannelPtr> GetChannel(const IClientRequestPtr& /*request*/) override
    {
        return GetChannel();
    }

    void Terminate(const TError& /*error*/) override
    { }

private:
    const TReplicationCardChannelConfigPtr Config_;
    const ICellDirectoryPtr CellDirectory_;
    const IReplicationCardResidencyCachePtr ReplicationCardResidencyCache_;
    const EPeerKind PeerKind_;
    const TReplicationCardId ReplicationCardId_;

    const TString EndpointDescription_;
    const IAttributeDictionaryPtr EndpointAttributes_;

    const TError UnavailableError_;

    const NLogging::TLogger Logger;

    TAtomicObject<TFuture<IChannelPtr>> ChannelFuture_;

    IChannelPtr Channel_;
    TCellTag CellTag_ = InvalidCellTag;
    YT_DECLARE_SPIN_LOCK(NThreading::TSpinLock, Lock_);

    TFuture<IChannelPtr> GetChannel()
    {
        if (auto future = ChannelFuture_.Load(); future && (!future.IsSet() || future.Get().IsOK())) {
            return future;
        }

        return CreateChannel();
    }

    void OnChannelFailed(const IChannelPtr& channel, const TError& error)
    {
        YT_LOG_DEBUG(error, "Replication card channel failed (IsUnavailable: %v)", IsUnavailableError(error));

        if (IsUnavailableError(error)) {
            auto cellTag = InvalidCellTag;

            if (auto guard = Guard(Lock_); channel == Channel_) {
                std::swap(cellTag,  CellTag_);
                Channel_.Reset();
            }

            if (cellTag != InvalidCellTag) {
                ReplicationCardResidencyCache_->ForceRefresh(ReplicationCardId_, cellTag);
                ChannelFuture_.Store(TFuture<IChannelPtr>());

                YT_LOG_DEBUG("Invalidated replication card cell tag from residency cache");
            }
        }
    }

    TFuture<IChannelPtr> CreateChannel()
    {
        YT_LOG_DEBUG("Creating new replication card channel");

        auto future = ReplicationCardResidencyCache_->GetReplicationCardResidency(ReplicationCardId_)
            .Apply(BIND(&TReplicationCardChannelProvider::OnReplicationCardResidencyFound, MakeStrong(this)));

        ChannelFuture_.Store(future);
        return future;
    }

    TFuture<IChannelPtr> OnReplicationCardResidencyFound(TCellTag cellTag)
    {
        if (auto channel = CellDirectory_->FindChannelByCellTag(cellTag, PeerKind_)) {
            auto detectingChannel = CreateFailureDetectingChannel(
                std::move(channel),
                Config_->RpcAcknowledgementTimeout, 
                BIND(&TReplicationCardChannelProvider::OnChannelFailed, MakeWeak(this)));

            {
                auto guard = Guard(Lock_);
                CellTag_ = cellTag;
                Channel_ = detectingChannel;
            }

            return MakeFuture<IChannelPtr>(detectingChannel);
        }

        return MakeFuture<IChannelPtr>(UnavailableError_);
    }

    bool IsUnavailableError(const TError& error)
    {
        if (IsChannelFailureError(error)) {
            return true;
        }
        auto code = error.GetCode();
        return code == NYT::EErrorCode::Timeout;
    }
};

////////////////////////////////////////////////////////////////////////////////

class TReplicationCardChannelFactory
    : public IReplicationCardChannelFactory
{
public:
    TReplicationCardChannelFactory(
        ICellDirectoryPtr cellDirectory,
        IReplicationCardResidencyCachePtr residencyCache,
        IChaosCellDirectorySynchronizerPtr synchronizer,
        TReplicationCardChannelConfigPtr config)
        : Config_(std::move(config))
        , CellDirectory_(std::move(cellDirectory))
        , ReplicationCardResidencyCache_(std::move(residencyCache))
        , Synchronizer_(std::move(synchronizer))
    { }

    NRpc::IChannelPtr CreateChannel(TReplicationCardId replicationCardId, EPeerKind peerKind) override
    {
        auto provider = New<TReplicationCardChannelProvider>(
            replicationCardId,
            CellDirectory_,
            ReplicationCardResidencyCache_,
            Synchronizer_,
            peerKind,
            Config_);
        return CreateRoamingChannel(std::move(provider));
    }

private:
    const TReplicationCardChannelConfigPtr Config_;
    const ICellDirectoryPtr CellDirectory_;
    const IReplicationCardResidencyCachePtr ReplicationCardResidencyCache_;
    const IChaosCellDirectorySynchronizerPtr Synchronizer_;
};

////////////////////////////////////////////////////////////////////////////////

IReplicationCardChannelFactoryPtr CreateReplicationCardChannelFactory(
    ICellDirectoryPtr cellDirectory,
    IReplicationCardResidencyCachePtr residencyCache,
    IChaosCellDirectorySynchronizerPtr synchronizer,
    TReplicationCardChannelConfigPtr config)
{
    return New<TReplicationCardChannelFactory>(
        std::move(cellDirectory),
        std::move(residencyCache),
        std::move(synchronizer),
        std::move(config));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NChaosClient