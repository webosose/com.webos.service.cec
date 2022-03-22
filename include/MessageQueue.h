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

#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <glib.h>
#include <functional>
#include <unistd.h>
#include <unordered_map>
#include "Logger.h"
#include "Command.h"
#include <nyx/nyx_client.h>


typedef std::function<void(CommandType, std::vector<std::string>)> MsgCallback;

struct MessageData
{
    CommandType type;
    std::unordered_map<std::string, std::string> params;
};

class MessageQueue
{
public:
    MessageQueue();
    ~MessageQueue();
    void addMessage(std::shared_ptr<MessageData>);
    void setCallback(MsgCallback);
    static void nyxCallback(nyx_cec_response_t *);

private:
    void dispatchMessage();
    bool handleMessage(std::shared_ptr<MessageData>);
    void init();
    void sendCommand(std::shared_ptr<MessageData>);
    void getConfig(std::shared_ptr<MessageData>);
    void setConfig(std::shared_ptr<MessageData>);


    std::vector<std::shared_ptr<MessageData>> mQueue;
    std::thread mThread;
    std::mutex mMutex;
    std::condition_variable mCondVar;
    bool mQuit;
    MsgCallback mCb;
    nyx_device_handle_t mDevice;

};

