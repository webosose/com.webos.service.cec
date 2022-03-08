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

#include <map>

#include "CeCErrors.h"

static std::map<CecErrorCode, std::string> cecErrorTextTable =
{
	{CEC_ERR_BAD_JSON, "Invalid JSON input"},
    {CEC_ERR_SCHEMA_VALIDATION_FAILED, "The JSON input does not match the expected schema"},
    {CEC_ERR_DESTADDR_PARAM_MISSING, "Required destination address parameter is missing"},
    {CEC_ERR_COMMAND_PARAM_MISSING, "Required command parameter is missing"},
    {CEC_ERR_CONFNAME_PARAM_MISSING, "Required parameter config name is missing"},
	{CEC_ERR_CONFVALUE_PARAM_MISSING, "Required parameter config value is missing"},
    {CEC_ERR_INVALID_PROVIDER_PAYLOAD, "Invalid payload received from provider"},
    {CEC_ERR_UNKNOWN_ERROR, "Unknown error"}
};

const std::string retrieveErrorText(CecErrorCode errorCode)
{
	return cecErrorTextTable[errorCode];
}
