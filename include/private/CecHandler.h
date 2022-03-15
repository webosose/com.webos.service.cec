// Copyright (c) 2019 LG Electronics, Inc.
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

#ifndef _CECHANDLER_H_
#define _CECHANDLER_H_

#include <list>
#include <string>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include "Command.h"


enum HandlerRank {
  LG_RANK,
  SAMSUNG_RANK,
  DEFAULT_RANK
};

class CecHandler {
public:

  CecHandler() {
  }
  virtual ~CecHandler(){}
  virtual bool HandleCommand(std::shared_ptr<Command> command) = 0;
  virtual HandlerRank GetRank() = 0;
  virtual std::string GetDeviceInfo(std::string destAddress) { return ""; }
};
#endif /* _CECHANDLER_H_ */
