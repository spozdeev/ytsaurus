#pragma once

#include <yt/yt/core/concurrency/async_stream.h>

#include <yt/yt/core/yson/public.h>

#include <library/cpp/yt/memory/ref.h>

#include <util/stream/input.h>
#include <util/stream/file.h>

namespace NYT::NCoreDump {

////////////////////////////////////////////////////////////////////////////////

class ISparseCoreDumpConsumer
    : public TRefCounted
{
public:
    virtual void OnRegularBlock(TSharedRef block) = 0;

    virtual void OnZeroBlock(i64 length) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class TFileSparseCoreDumpWriter
    : public ISparseCoreDumpConsumer
{
public:
    TFileSparseCoreDumpWriter(TFile* outputFile);

    ~TFileSparseCoreDumpWriter();

    void OnRegularBlock(TSharedRef block) override;

    void OnZeroBlock(i64 length) override;

private:
    i64 FileOffset_ = 0;
    TFile* OutputFile_;
};

////////////////////////////////////////////////////////////////////////////////

class TStreamSparseCoreDumpWriter
    : public ISparseCoreDumpConsumer
{
public:
    TStreamSparseCoreDumpWriter(NConcurrency::IAsyncOutputStreamPtr outputStream, TDuration writeTimeout = TDuration::Max());

    void OnRegularBlock(TSharedRef block) override;

    void OnZeroBlock(i64 length) override;

    const static TSharedRef ZeroBlockHeader;
    const static TSharedRef RegularBlockHeader;

    NConcurrency::IAsyncZeroCopyOutputStreamPtr OutputStream_;
    const TDuration WriteTimeout_;
};

////////////////////////////////////////////////////////////////////////////////

i64 SparsifyCoreDump(
    NConcurrency::IAsyncInputStreamPtr coreDumpStream,
    TIntrusivePtr<ISparseCoreDumpConsumer> consumer,
    TDuration readTimeout = TDuration::Max());

i64 WriteSparseCoreDump(IInputStream* in, TFile* out);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NCoreDump
