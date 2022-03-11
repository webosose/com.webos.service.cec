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
#include <memory>
#include <unordered_map>
#include <list>
#include <map>

#include <luna-service2/lunaservice.hpp>

#include "Logger.h"
#include "Command.h"
#include "IScanObserver.h"

class CECLunaService: public LS::Handle, public IScanObserver {
public:
    CECLunaService();

    virtual ~CECLunaService();

    void run();

    void registerMethods();

    void stop();

    bool listAdapters(LSMessage &message);

    bool scan(LSMessage &message);

    bool sendCommand(LSMessage &message);

    bool getConfig(LSMessage &message);

    bool setConfig(LSMessage &message);

    void notifyScanStatus(std::shared_ptr<ScanResData> data);

    static void callback(void *ctx, uint16_t clientId, enum CommandType type, std::shared_ptr<CommandResData> respData);
private:

    void handleListAdapters(pbnjson::JValue &requestObj);
    void handleScan(pbnjson::JValue &requestObj);
    void handleSendCommand(pbnjson::JValue &requestObj);
    void handleGetConfig(pbnjson::JValue &requestObj);
    void handleSetConfig(pbnjson::JValue &requestObj);
    void parseResponseObject(pbnjson::JValue &responseObj, enum CommandType type,
            std::shared_ptr<CommandResData> respData);
    using MainLoopT = std::unique_ptr<GMainLoop, void (*)(GMainLoop *)>;
    MainLoopT main_loop_ptr;
    std::list<LS::Call> callObjects;
    LS::Handle *luna_handle;
    std::list<LS::Message> getTimeClients;
    std::map<uint16_t, LSMessage*> m_clients;
    uint16_t m_clientId = 0;

    LS::SubscriptionPoint mScanSubscriptions;
};
