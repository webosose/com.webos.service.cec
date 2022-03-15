// Copyright (c) 2019-2021 LG Electronics, Inc.
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

#ifndef _DEFAULTCECHANDLER_H_
#define _DEFAULTCECHANDLER_H_

#include <vector>
#include "CecHandler.h"
#include "CecController.h"
#include "MessageQueue.h"

class DefaultCecHandler : public CecHandler
{
  private:
    static bool mIsObjRegistered;
    HandlerRank mRank = DEFAULT_RANK;
    std::list<std::shared_ptr<Command>> cmdList;

    DefaultCecHandler();

    //Register Object to object factory. This is called automatically
    static bool RegisterObject() {
      return (CecController::getInstance()->Register( &DefaultCecHandler::CreateObject,
                                                      DEFAULT_RANK));
    }

    bool HandleSendCommand(std::shared_ptr<Command> command);
    bool HandleScan(std::shared_ptr<Command> command);
    bool HandleListAdapters(std::shared_ptr<Command> command);
    bool HandleGetConfig(std::shared_ptr<Command> command);
    bool HandleSetConfig(std::shared_ptr<Command> command);

    static void HandleSendCommandCb(std::vector<std::string> response);

    MessageQueue mQueue;

  public:
    ~DefaultCecHandler();

    static CecHandler* CreateObject() {
      if (mIsObjRegistered) {
        AppLogInfo()<<"Registering DefaultCecHandler";
        return new DefaultCecHandler();
      } else {
        return nullptr;
      }
    }
    bool HandleCommand(std::shared_ptr<Command> command);
    std::string GetDeviceInfo(std::string destAddress);
    HandlerRank GetRank() { return mRank; }
};

#endif // _DEFAULTCECHANDLER_H
