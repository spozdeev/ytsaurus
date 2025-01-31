//
// Copyright 2020 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef GRPC_CORE_LIB_SECURITY_CREDENTIALS_EXTERNAL_FILE_EXTERNAL_ACCOUNT_CREDENTIALS_H
#define GRPC_CORE_LIB_SECURITY_CREDENTIALS_EXTERNAL_FILE_EXTERNAL_ACCOUNT_CREDENTIALS_H

#include <grpc/support/port_platform.h>

#include <functional>
#include <util/generic/string.h>
#include <util/string/cast.h>
#include <vector>

#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/error.h"
#include "src/core/lib/security/credentials/external/external_account_credentials.h"

namespace grpc_core {

class FileExternalAccountCredentials final : public ExternalAccountCredentials {
 public:
  static RefCountedPtr<FileExternalAccountCredentials> Create(
      Options options, std::vector<TString> scopes,
      grpc_error_handle* error);

  FileExternalAccountCredentials(Options options,
                                 std::vector<TString> scopes,
                                 grpc_error_handle* error);

 private:
  void RetrieveSubjectToken(
      HTTPRequestContext* ctx, const Options& options,
      std::function<void(TString, grpc_error_handle)> cb) override;

  // Fields of credential source
  TString file_;
  TString format_type_;
  TString format_subject_token_field_name_;
};

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_SECURITY_CREDENTIALS_EXTERNAL_FILE_EXTERNAL_ACCOUNT_CREDENTIALS_H
