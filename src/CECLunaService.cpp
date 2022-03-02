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

#include <CECLunaService.h>
#include <glib/glist.h>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.h>
#include <luna-service2++/error.hpp>
#include <luna-service2++/message.hpp>
#include <string>

const std::string SERVICE_NAME = "com.webos.service.cec";

CECLunaService::CECLunaService()
: main_loop_ptr(g_main_loop_new(nullptr, false), g_main_loop_unref), LS::Handle(LS::registerService(SERVICE_NAME.c_str()))
{
    registerMethods();
}

CECLunaService::~CECLunaService()
{

}

void CECLunaService::registerMethods() {

    static const LSMethod methods[] = {
        {"listDevices", &LS::Handle::methodWraper<CECLunaService, &CECLunaService::listDevices>, static_cast<LSMethodFlags>(LUNA_METHOD_FLAG_VALIDATE_IN)},
        {"scan", &LS::Handle::methodWraper<CECLunaService, &CECLunaService::scan>, static_cast<LSMethodFlags>(LUNA_METHOD_FLAG_VALIDATE_IN)},
        {"sendMessage", &LS::Handle::methodWraper<CECLunaService, &CECLunaService::sendMessage>, static_cast<LSMethodFlags>(LUNA_METHOD_FLAG_VALIDATE_IN)},
        {nullptr, nullptr}
    };

    luna_handle->registerCategory("/", methods, nullptr, nullptr);
    luna_handle->setCategoryData("/", this);
}

void CECLunaService::run()
{
    g_main_loop_run(main_loop_ptr.get());
}

void CECLunaService::stop()
{
    g_main_loop_quit(main_loop_ptr.get());
}

bool CECLunaService::receiveCallback(LSHandle *sh, LSMessage *pMessage, void *pCtx)
{
    return true;
}


bool CECLunaService::listDevices(LSMessage &ls_message) {
    LS::Message request(&ls_message);
    pbnjson::JValue responseJson;
    pbnjson::JValue parsed = pbnjson::JDomParser::fromString(request.getPayload());
    if (parsed.isError()) {
        responseJson = pbnjson::JObject { { "returnValue", false }, { "errorText", "Invalid parameter" }, { "errorCode", 1 } };
        request.respond(responseJson.stringify().c_str());
        return true;
    } else {
        responseJson = pbnjson::JObject { { "returnValue", true }, { "message", "test response" }};
        request.respond(responseJson.stringify().c_str());
    }
    return true;
}

bool CECLunaService::scan(LSMessage &ls_message) {
    LS::Message request(&ls_message);
    pbnjson::JValue responseJson;
    pbnjson::JValue parsed = pbnjson::JDomParser::fromString(request.getPayload());
    if (parsed.isError()) {
        responseJson = pbnjson::JObject { { "returnValue", false }, { "errorText", "Invalid parameter" }, { "errorCode", 1 } };
        request.respond(responseJson.stringify().c_str());
        return true;
    } else {
        responseJson = pbnjson::JObject { { "returnValue", true }, { "message", "test response" }};
        request.respond(responseJson.stringify().c_str());
    }
    return true;
}

bool CECLunaService::sendMessage(LSMessage &ls_message) {
    LS::Message request(&ls_message);
    pbnjson::JValue responseJson;
    pbnjson::JValue parsed = pbnjson::JDomParser::fromString(request.getPayload());
    if (parsed.isError()) {
        responseJson = pbnjson::JObject { { "returnValue", false }, { "errorText", "Invalid parameter" }, { "errorCode", 1 } };
        request.respond(responseJson.stringify().c_str());
        return true;
    } else {
        responseJson = pbnjson::JObject { { "returnValue", true }, { "message", "test response" }};
        request.respond(responseJson.stringify().c_str());
    }
    return true;
}

