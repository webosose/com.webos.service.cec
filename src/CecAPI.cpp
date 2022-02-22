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

#include "Logger.h"
#include <unistd.h>
#include <string>
#include <locale>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <regex>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "CecAPI.h"

CecService::CecService(LS::Handle *ls_handle)
: main_loop_ptr(g_main_loop_new(nullptr, false), g_main_loop_unref),
  luna_handle(ls_handle)
{
}

CecService::~CecService() {
}

void CecService::run() {
    // attach to main loop and start running
    g_main_loop_run(main_loop_ptr.get());
}

void CecService::stop() { g_main_loop_quit(main_loop_ptr.get()); }

bool CecService::receiveCallback(LSHandle *sh, LSMessage *pMessage, void *pCtx)
{
    return true;
}


bool CecService::ListGpio(LSMessage &ls_message) {
    LS::Message request(&ls_message);
    bool subscription = false;
    pbnjson::JValue response_json;
    pbnjson::JValue parsed = pbnjson::JDomParser::fromString(request.getPayload());
    if (parsed.isError()) {
        response_json =
                pbnjson::JObject{{"returnValue", false}, {"errorText", "Failed to parse params"}, {"errorCode", 1}};
        request.respond(response_json.stringify().c_str());
        return false;
    } else {
        std::string temp;
        bool extra_property = false;
        for(auto ii:parsed)
        {
            if(ii.first.asString() == "subscribe")
            {
                continue;
            }
            else
            {
                extra_property = true;
                temp = ii.first.asString();
                response_json = pbnjson::JObject{{"returnValue", false},{"errorText", temp+" property not allowed"}};
            }
        }
        if(extra_property == true)
        {
            request.respond(response_json.stringify().c_str());
            return true;
        }
        else {
            try {
            pbnjson::JValue gpioList = pbnjson::JArray();
            subscription = parsed["subscribe"].asBool();
            response_json =
                    pbnjson::JObject{
                {"returnValue", true},
                {"subscribed", subscription},
                {"gpioList", gpioList}};
            }
            catch (LS::Error &err) {
                response_json = pbnjson::JObject{{"returnValue", false}, {"errorText", err.what()}};
            }
            request.respond(response_json.stringify().c_str());
        }
    }
    return true;
}

// Private Methods
void CecService::registerMethodsToLsHub() {
    static const LSMethod gpio[] = {
        {"list", &LS::Handle::methodWraper<CecService, &CecService::ListGpio>,
        static_cast<LSMethodFlags>(LUNA_METHOD_FLAG_VALIDATE_IN)},
        {nullptr, nullptr}};

    luna_handle->registerCategory("/", gpio, nullptr, nullptr);
    luna_handle->setCategoryData("/", this);

}
