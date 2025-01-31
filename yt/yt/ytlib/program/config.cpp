#include "config.h"

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

void TNativeSingletonsConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("chunk_client_dispatcher", &TThis::ChunkClientDispatcher)
        .DefaultNew();
    registrar.Parameter("native_authentication_manager", &TThis::NativeAuthenticationManager)
        .DefaultNew();
}

////////////////////////////////////////////////////////////////////////////////

void TNativeSingletonsDynamicConfig::Register(TRegistrar registrar)
{
    registrar.Parameter("chunk_client_dispatcher", &TThis::ChunkClientDispatcher)
        .DefaultNew();
    registrar.Parameter("native_authentication_manager", &TThis::NativeAuthenticationManager)
        .DefaultNew();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
