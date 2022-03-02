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

#pragma once

#include <glib.h>
#include <luna-service2++/handle.hpp>
#include <memory>
#include <unordered_map>
#include <list>
#include <Logger.h>

class CECLunaService :public LS::Handle
{
public:
    CECLunaService();

    virtual ~CECLunaService();

    void run();

    void registerMethods();

    void stop();

    bool listDevices(LSMessage &ls_message);

    bool scan(LSMessage &ls_message);

    bool sendMessage(LSMessage &ls_message);

    static bool receiveCallback(LSHandle *sh, LSMessage *reply, void *ctx);
private:
    using MainLoopT = std::unique_ptr<GMainLoop, void (*)(GMainLoop *)>;
    MainLoopT main_loop_ptr;
    std::list<LS::Call> callObjects;
    LS::Handle *luna_handle;
    std::list<LS::Message> getTimeClients;
};
