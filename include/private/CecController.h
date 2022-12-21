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

#ifndef _CECCONTROLLER_H_
#define _CECONTROLLER_H_

#include <list>
#include <string>
#include <algorithm>
#include <functional>
#include <utility>
#include <memory>
#include <vector>
#include "Command.h"
#include "MessageQueue.h"
#include "CecHandler.h"
#include <future>

typedef CecHandler* (*CreateCecHandlerObject)();

class CecController {
protected:
  CecController();
  CecController(const CecController&) = delete;
  CecController& operator=(const CecController&) = delete;

  std::list<CecHandler*> mHandlerList;
  std::list<std::pair<CreateCecHandlerObject, HandlerRank>> mCreatorList;
  bool mInitlialized = false;

  static CecController *mInstance;
public:

  static CecController* getInstance();

  virtual ~CecController();
  bool initialize();
  virtual bool HandleCommand(std::shared_ptr<Command> command);
  virtual bool Register(CreateCecHandlerObject createObject, HandlerRank rank);
  virtual std::shared_ptr<CecDevice> GetDeviceInfo(std::string destAddress);
  std::future<bool> m_InitFut;
};
#endif /* _CECHANDLER_H_ */
