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

#include "CecController.h"

CecController* CecController::mInstance=nullptr;

CecController* CecController::getInstance() {

  if (!mInstance) {
    mInstance = new CecController();
  }
  return mInstance;
}

CecController::CecController() {
  AppLogDebug()<<" CecController::"<<__func__<<":"<<__LINE__;
}

CecController::~CecController() {
  mCreatorList.clear();
  mHandlerList.clear();
}

bool CecController::initialize() {
  AppLogInfo()<<" CecController::"<<__func__<<":"<<__LINE__;
  CecHandler *ptr = nullptr;
  for (auto it = mCreatorList.begin(); it!=mCreatorList.end(); ++it) {
    ptr = (*it).first();
    mHandlerList.push_back(ptr);
    ptr = nullptr;
  }
  return true;
}

bool CecController::HandleCommand(std::shared_ptr<Command> command) {
  AppLogInfo()<<" CecController::"<<__func__<<":"<<__LINE__;

  if(!mInitlialized) {
    initialize();
    mInitlialized = true;
  }

  for (auto it = mHandlerList.begin(); it!=mHandlerList.end(); ++it) {
    AppLogDebug()<<"CecController::"<<__func__<<":"<<__LINE__<<" Calling registered handlers";
    if((*it)->HandleCommand(command) == true)
      return true;
  }
  return false;
}

bool CecController::Register(CreateCecHandlerObject createObject, HandlerRank rank) {
  AppLogInfo()<<" CecController::"<<__func__<<":"<<__LINE__<<" Rank:"<<rank;
  std::pair<CreateCecHandlerObject,HandlerRank> creator;

  creator.first = createObject;
  creator.second = rank;
  if (mCreatorList.size()) {
    auto it = mCreatorList.begin();
    for (; it!=mCreatorList.end(); ++it) {
      if((*it).second > rank) {
        AppLogDebug()<<" CecController::"<<__func__<<":"<<__LINE__<<" Inserting Handler";
        mCreatorList.insert(it, creator);
        break;
      }
    }
    if (it == mCreatorList.end()) {
      AppLogDebug()<<" CecController::"<<__func__<<":"<<__LINE__<<" Inserting Handler";
      mCreatorList.push_back(creator);
    }
    return true;
  }
  mCreatorList.push_back(creator);
  return true;
}

std::shared_ptr<CecDevice> CecController::GetDeviceInfo(std::string destAddress) {
  CecHandler *default_handler = mHandlerList.back();
  return default_handler->GetDeviceInfo(destAddress);
}
