#pragma once

#include "public.h"

#include <yt/yt/core/misc/arithmetic_formula.h>

#include <yt/yt/core/ytree/yson_serializable.h>
#include <yt/yt/core/ytree/yson_struct.h>

namespace NYT::NTabletBalancer {

////////////////////////////////////////////////////////////////////////////////

class TBundleTabletBalancerConfig
    : public NYTree::TYsonSerializable
{
public:
    bool EnableInMemoryCellBalancer;
    bool EnableCellBalancer;
    bool EnableTabletSizeBalancer;

    bool EnableTabletCellSmoothing;

    double HardInMemoryCellBalanceThreshold;
    double SoftInMemoryCellBalanceThreshold;

    i64 MinTabletSize;
    i64 MaxTabletSize;
    i64 DesiredTabletSize;

    i64 MinInMemoryTabletSize;
    i64 MaxInMemoryTabletSize;
    i64 DesiredInMemoryTabletSize;

    double TabletToCellRatio;

    TTimeFormula TabletBalancerSchedule;

    bool EnableVerboseLogging;

    TBundleTabletBalancerConfig();
};

DEFINE_REFCOUNTED_TYPE(TBundleTabletBalancerConfig)

////////////////////////////////////////////////////////////////////////////////

class TTableTabletBalancerConfig
    : public NYTree::TYsonSerializable
{
public:
    bool EnableAutoReshard;
    bool EnableAutoTabletMove;

    std::optional<i64> MinTabletSize;
    std::optional<i64> MaxTabletSize;
    std::optional<i64> DesiredTabletSize;
    std::optional<int> DesiredTabletCount;
    std::optional<int> MinTabletCount;
    bool EnableVerboseLogging;

    TTableTabletBalancerConfig();

    // COMPAT(ifsmirnov)
    void SetMinTabletSize(std::optional<i64> value);

    void SetDesiredTabletSize(std::optional<i64> value);
    void SetMaxTabletSize(std::optional<i64> value);

private:
    void CheckTabletSizeInequalities() const;

    void SetTabletSizeConstraint(std::optional<i64>* member, std::optional<i64> value);
};

DEFINE_REFCOUNTED_TYPE(TTableTabletBalancerConfig)

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NTabletBalancer