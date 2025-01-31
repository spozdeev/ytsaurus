#pragma once

#include "private.h"

#include <yt/yt/server/master/cell_master/public.h>

#include <yt/yt/server/master/object_server/public.h>

namespace NYT::NSecurityServer {

////////////////////////////////////////////////////////////////////////////////

NObjectServer::IObjectProxyPtr CreateUserProxy(
    NCellMaster::TBootstrap* bootstrap,
    NObjectServer::TObjectTypeMetadata* metadata,
    TUser* user);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NSecurityServer
