// Copyright (c) 2022 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0


#ifndef CEC_ERRORS_H_
#define CEC_ERRORS_H_

#include <string>

enum CecErrorCode
{
	CEC_ERR_BAD_JSON = 1,
    CEC_ERR_SCHEMA_VALIDATION_FAILED,
    CEC_ERR_DESTADDR_PARAM_MISSING,
    CEC_ERR_COMMAND_PARAM_MISSING,
    CEC_ERR_CONFNAME_PARAM_MISSING,
    CEC_ERR_CONFVALUE_PARAM_MISSING,
    CEC_ERR_INVALID_PROVIDER_PAYLOAD,
    CEC_ERR_UNKNOWN_ERROR
};

const std::string retrieveErrorText(CecErrorCode errorCode);

#endif //CEC_ERRORS_H_
