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

#include "MessageQueue.h"

static MessageQueue *objPtr;
CommandType mType;

MessageQueue::MessageQueue()
    : mQuit(false)
{
    objPtr = this;
    init();
    mThread = std::thread(std::bind(&MessageQueue::dispatchMessage, this));
}

MessageQueue::~MessageQueue()
{
    mQuit = true;
    mCondVar.notify_one();
    if (mThread.joinable())
    {
        mThread.join();
    }
    mQueue.clear();
    nyx_device_close(mDevice);
    nyx_deinit();
}

void MessageQueue::init()
{
    nyx_error_t error = nyx_init();
    if (NYX_ERROR_NONE == error)
    {
        error = nyx_device_open(NYX_DEVICE_CEC, "Main", &mDevice);
        if ((NYX_ERROR_NONE != error) || (NULL == mDevice))
        {
            AppLogError() <<"Failed to get  Open nyx device: "<< error;
        }
        AppLogDebug() <<"Open nyx device: Success \n";
        nyx_cec_callbacks_t *objCb = new nyx_cec_callbacks_t;
        objCb->response_cb = &nyxCallback;
        nyx_cec_set_callback(mDevice, objCb);
    }
}

void MessageQueue::setCallback(MsgCallback cb)
{
    mCb = std::move(cb);
}

void MessageQueue::nyxCallback(nyx_cec_response_t *response)
{
    AppLogDebug() <<__func__ << "Received :\n";
    std::vector<std::string> resp;
    for(int i=0;i<response->size;i++)
    {
        AppLogDebug() <<response->responses[i]<<"\n";
        resp.push_back(response->responses[i]);
    }
    objPtr->mCb(mType,resp);
}

void MessageQueue::sendCommand(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    mType = request->type;

    nyx_error_t error;
    nyx_cec_command_t command;
    command.size = request->params.size();
    if(request->type == CommandType::SCAN)
        strcpy(command.name,"scan");
    else if(request->type == CommandType::LIST_ADAPTERS)
        strcpy(command.name,"listAdapters");
    else if(request->type == CommandType::SEND_COMMAND)
        strcpy(command.name,request->params["cmd-name"].c_str());
    AppLogDebug() <<"COMMAND NAME : [ "<<command.name<<" ]"<<"\n";
    int i =0;
    for(auto it : request->params) {
        strcpy(command.params[i].name,it.first.c_str());
        if (!it.second.empty())
          strcpy(command.params[i].value,it.second.c_str());
        else
          strcpy(command.params[i].value," ");
        AppLogDebug() <<"Name : [ "<<command.params[i].name<<" ]" <<"Value : ["<<command.params[i].value<<" ]"<<"\n";
        i++;
    }
    error = nyx_cec_send_command(mDevice, &command);
    if(error == NYX_ERROR_NOT_IMPLEMENTED)
    {
        AppLogDebug() <<__func__<<": NYX_ERROR_NOT_IMPLEMENTED\n";
        std::vector<std::string> resp;
        std::string reply = "response: success";
        resp.push_back(reply);
        mCb(mType,resp);
    }
    else if(error != NYX_ERROR_NONE)
    {
        AppLogError() <<__func__<<": Failed with :"<<error<<"\n";
        std::vector<std::string> resp;
        std::string reply = "response: failed";
        resp.push_back(reply);
        mCb(mType,resp);
    }
}

void MessageQueue::getConfig(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    mType = request->type;

    nyx_error_t error;
    char *configName = nullptr;
    char *value = new char[100];

    for(auto it : request->params) {
        AppLogDebug() <<__func__<<" Updating param"<<"\n";
        if(it.first != "adapter") {
            configName = new char[it.first.size() + 1];
            strcpy(configName,it.first.c_str());
        }
    }

    error = nyx_cec_get_config(mDevice, configName, &value);
    if(error != NYX_ERROR_NONE)
    {
        AppLogError() <<__func__<<": Failed with :"<<error<<"\n";
        std::vector<std::string> resp;
        std::string reply = "response: failed";
        resp.push_back(reply);
        mCb(mType,resp);
    }
    else {
        AppLogDebug() <<__func__<<": Value :"<<value<<"\n";
        std::vector<std::string> resp;
        resp.push_back(value);
        mCb(mType,resp);
    }
    if (configName != nullptr)
      delete[] configName;
    delete[] value;
}

void MessageQueue::setConfig(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    mType = request->type;

    nyx_error_t error;
    char *type = nullptr;
    char *value = nullptr;
    for (auto it : request->params) {
        if(it.first != "adapter") {
            type = new char[it.first.size()+1];
            strcpy(type,it.first.c_str());
            value = new char[it.second.size()+1];
            strcpy(value,it.second.c_str());
        }
    }
    error = nyx_cec_set_config(mDevice, type, value);
    if((error == NYX_ERROR_NOT_IMPLEMENTED) || (error == NYX_ERROR_NONE))
    {
        AppLogDebug() <<__func__<<": NYX_ERROR_NOT_IMPLEMENTED s\n";
        std::vector<std::string> resp;
        std::string reply = "response: success";
        resp.push_back(reply);
        mCb(mType,resp);
    }
    else  if (NYX_ERROR_NONE != error)
    {
        AppLogError() <<__func__<<": Failed with :"<<error<<"\n";
        std::vector<std::string> resp;
        std::string reply = "response: failed";
        resp.push_back(reply);
        mCb(mType,resp);
    }

    if (type != nullptr)
      delete[] type;
    if (value != nullptr)
      delete[] value;
}

bool MessageQueue::handleMessage(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__ << "\n";
    switch(request->type)
    {
        case CommandType::LIST_ADAPTERS:
        {
            AppLogDebug() <<__func__<<":LIST_ADAPTERS MessageType\n";
            sendCommand(request);
        }
        break;
        case CommandType::SCAN:
        {
            AppLogDebug() <<__func__<<":SCAN MessageType\n";
            sendCommand(request);
        }
        break;
        case CommandType::SEND_COMMAND:
        {
            AppLogDebug() <<__func__<<":SEND_COMMAND MessageType\n";
            sendCommand(request);
        }
        break;
        case CommandType::GET_CONFIG:
        {
            AppLogDebug() <<__func__<<":GET_CONFIG MessageType\n";
            getConfig(request);
        }
        break;
        case CommandType::SET_CONFIG:
        {
            AppLogDebug() <<__func__<<":SET_CONFIG MessageType\n";
            setConfig(request);
        }
        break;
        default:
            AppLogDebug() <<__func__<<": UNKNOWN MessageType\n";
        break;
    }
    return true;
}

void MessageQueue::addMessage(std::shared_ptr<MessageData> request)
{
    AppLogInfo() <<__func__ << " called \n";
    std::unique_lock < std::mutex > lock(mMutex);
    mQueue.push_back(request);
    lock.unlock();
    mCondVar.notify_one();
}

void MessageQueue::dispatchMessage()
{
    AppLogDebug() <<__func__ << " called \n";
    std::unique_lock < std::mutex > lock(mMutex);
    do {
        mCondVar.wait(lock, [this] {
            return (mQueue.size() || mQuit);
        });
        if (mQueue.size() && !mQuit)
        {
            std::shared_ptr<MessageData> front = std::move(mQueue.front());
            mQueue.erase(mQueue.begin());
            lock.unlock();
            handleMessage(front);
            lock.lock();
        }
        else
            lock.unlock();
    } while (!mQuit);
}

