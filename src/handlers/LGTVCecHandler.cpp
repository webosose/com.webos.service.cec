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

#include "LGTVCecHandler.h"

bool LGTVCecHandler::mIsObjRegistered = LGTVCecHandler::RegisterObject();

LGTVCecHandler::LGTVCecHandler() :
                       CecHandler() {
}

LGTVCecHandler::~LGTVCecHandler() {
}

bool LGTVCecHandler::HandleCommand(std::shared_ptr<Command> command) {
  AppLogInfo()<<" LGTVCecHandler::"<<__func__<<":"<<__LINE__;

  if (command->getType() != SEND_COMMAND) {
    return false;
  }

  std::shared_ptr<CommandReqData> reqData = command->getData();

  std::shared_ptr<SendCommandReqData> commandData(reqData, static_cast<SendCommandReqData*>(reqData.get()));

  std::shared_ptr<CecDevice> device = CecController::getInstance()->GetDeviceInfo(commandData->destAddress);
  if (device != nullptr) {
    if (device->name == "TV" && device->vendor == "LG") {
      //TODO: Modify the command as per LG requirement
    }
  }
  return false;
}

