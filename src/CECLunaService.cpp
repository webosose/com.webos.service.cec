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

#include <pbnjson.hpp>

#include "CeCErrors.h"
#include "ls2utils.h"
#include "CECLunaService.h"

const std::string SERVICE_NAME = "com.webos.service.cec";
const std::string GET_STATUS_INVALID_PAYLOAD = "Invalid Payload";

CECLunaService::CECLunaService() :
        main_loop_ptr(g_main_loop_new(nullptr, false), g_main_loop_unref), LS::Handle(SERVICE_NAME.c_str()) {
    registerMethods();
}

CECLunaService::~CECLunaService() {

}

void CECLunaService::registerMethods() {

    //Register Luna Methods for CeC service
    LS_CREATE_CATEGORY_BEGIN(CECLunaService, base) LS_CATEGORY_CLASS_METHOD(CECLunaService, listAdapters)
    LS_CATEGORY_CLASS_METHOD(CECLunaService, scan)
    LS_CATEGORY_CLASS_METHOD(CECLunaService, sendCommand)
    LS_CATEGORY_CLASS_METHOD(CECLunaService, getConfig)
    LS_CATEGORY_CLASS_METHOD(CECLunaService, setConfig)
    LS_CREATE_CATEGORY_END

    registerCategory("/", LS_CATEGORY_TABLE_NAME(base), NULL, NULL);
    setCategoryData("/", this);
    mScanSubscriptions.setServiceHandle(this);
}

void CECLunaService::run() {
    attachToLoop(main_loop_ptr.get());
    g_main_loop_run(main_loop_ptr.get());
}

void CECLunaService::stop() {
    g_main_loop_quit(main_loop_ptr.get());
}

bool CECLunaService::listAdapters(LSMessage &message) {

    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema = SCHEMA_ANY;

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::listAdapters code: " << parseError << "\n";
        if (JSON_PARSE_SCHEMA_ERROR != parseError)
            LSUtils::respondWithError(request, CEC_ERR_BAD_JSON);
        else
            LSUtils::respondWithError(request, CEC_ERR_SCHEMA_VALIDATION_FAILED);
        return true;
    } else {
        //Create Command and send to CEC Controller
        LSMessage *requestMessage = request.get();
        LSMessageRef(requestMessage);
        m_clients[++m_clientId] = requestMessage;
        handleListAdapters(requestObj);
        return true;
    }
}

void CECLunaService::handleListAdapters(pbnjson::JValue &requestObj) {

    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::LIST_ADAPTERS, std::bind(&CECLunaService::callback, this, m_clientId,
                    CommandType::LIST_ADAPTERS, std::placeholders::_1));
    //Send command to CEC Controller
    //Create sample data and send the response
    std::shared_ptr<ListAdaptersResData> respData = std::make_shared<ListAdaptersResData>();
    std::list < std::string > my_list = { "/dev/cec0", "/dev/cec1" };
    respData->cecAdapters = my_list;
    respData->returnValue = true;
    CECLunaService::callback(this, m_clientId, CommandType::LIST_ADAPTERS, respData);
}

bool CECLunaService::scan(LSMessage &message) {
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema = SCHEMA_ANY;

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::scan code: " << parseError << "\n";
        if (JSON_PARSE_SCHEMA_ERROR != parseError)
            LSUtils::respondWithError(request, CEC_ERR_BAD_JSON);
        else
            LSUtils::respondWithError(request, CEC_ERR_SCHEMA_VALIDATION_FAILED);
        return true;
    } else {

        //Create Command and send to CEC Controller
        LSMessage *requestMessage = request.get();
        LSMessageRef(requestMessage);
        m_clients[++m_clientId] = requestMessage;
        handleScan(requestObj);

        // Handle scan subscription
        if (request.isSubscription()) {
            LS::SubscriptionPoint *subscriptionPoint = new LS::SubscriptionPoint;
            mScanSubscriptions.subscribe(request);
        }
        return true;
    }
}

void CECLunaService::handleScan(pbnjson::JValue &requestObj) {

    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::SCAN, std::bind(&CECLunaService::callback, this, m_clientId, CommandType::SCAN,
                    std::placeholders::_1));

    std::shared_ptr<ScanReqData> data = std::make_shared<ScanReqData>();
    if (requestObj.hasKey("subscribe")) {
        data->subscribed = requestObj["subscribe"].asBool();
    }

    if (requestObj.hasKey("adapter")) {
        data->adapter = requestObj["adapter"].asString();
    }
    command->setData(data);
    //Send command to CEC Controller
    //Create sample data and send the response
    std::shared_ptr<ScanResData> respData = std::make_shared<ScanResData>();
    respData->devices.push_back(CecDevice("SmartTV", "0.0.0.0", "no", "LG", "OLED SmartTV", "1.3a", "on", "eng"));
    respData->devices.push_back(CecDevice("Recorder", "1.0.0.0", "yes", "LG", "CECTester", "1.3b", "on", "frh"));
    respData->returnValue = true;
    CECLunaService::callback(this, m_clientId, CommandType::SCAN, respData);
}

bool CECLunaService::sendCommand(LSMessage &message) {
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema =
            STRICT_SCHEMA(
                    PROPS_5(PROP(device, string), PROP(srcAddress, string), PROP(destAddress, string), PROP(timeout, integer), OBJECT(command, OBJSCHEMA_2(PROP(name, string),OBJARRAY(args, OBJSCHEMA_2(PROP(arg, string), PROP(value, string)))))) REQUIRED_3(srcAddress, destAddress, command));

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::sendCommand code: " << parseError << "\n";
        if (JSON_PARSE_SCHEMA_ERROR != parseError)
            LSUtils::respondWithError(request, CEC_ERR_BAD_JSON);
        else if (!requestObj.hasKey("srcAddress"))
            LSUtils::respondWithError(request, CEC_ERR_SRCADDR_PARAM_MISSING);
        else if (!requestObj.hasKey("destAddress"))
            LSUtils::respondWithError(request, CEC_ERR_DESTADDR_PARAM_MISSING);
        else if (!requestObj.hasKey("command"))
            LSUtils::respondWithError(request, CEC_ERR_COMMAND_PARAM_MISSING);
        else
            LSUtils::respondWithError(request, CEC_ERR_SCHEMA_VALIDATION_FAILED);
        return true;
    } else {
        //Create Command and send to CEC Controller
        LSMessage *requestMessage = request.get();
        LSMessageRef(requestMessage);
        m_clients[++m_clientId] = requestMessage;
        handleSendCommand(requestObj);
        return true;
    }
}

void CECLunaService::handleSendCommand(pbnjson::JValue &requestObj) {

    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::SEND_COMMAND, std::bind(&CECLunaService::callback, this, m_clientId,
                    CommandType::SEND_COMMAND, std::placeholders::_1));

    std::shared_ptr<SendCommandReqData> data = std::make_shared<SendCommandReqData>();
    if (requestObj.hasKey("device")) {
        data->device = requestObj["device"].asString();
    }
    if (requestObj.hasKey("srcAddress")) {
        data->srcAddress = requestObj["srcAddress"].asString();
    }
    if (requestObj.hasKey("timeout")) {
        data->timeout = requestObj["timeout"].asNumber<int32_t>();
    }

    data->destAddress = requestObj["destAddress"].asString();
    CecCommand ceccommand;
    auto cecCommandObj = requestObj["command"];
    ceccommand.name = cecCommandObj["name"].asString();
    if (cecCommandObj.hasKey("args")) {
        auto cecCommandArgsObj = requestObj["args"];
        CecCommandArgs commandArgs;
        commandArgs.arg = cecCommandArgsObj["name"].asString();
        commandArgs.value = cecCommandArgsObj["value"].asString();
        ceccommand.args = commandArgs;
    }
    data->command = ceccommand;

    command->setData(data);
    //Send command to CEC Controller
    //Create sample data and send the response
    std::shared_ptr<CommandResData> respData = std::make_shared<CommandResData>();
    respData->returnValue = true;
    CECLunaService::callback(this, m_clientId, CommandType::SEND_COMMAND, respData);
}

bool CECLunaService::getConfig(LSMessage &message) {
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema = STRICT_SCHEMA(PROPS_1(PROP(key, string)) REQUIRED_1(key));

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::getConfig code: " << parseError << "\n";
        if (JSON_PARSE_SCHEMA_ERROR != parseError)
            LSUtils::respondWithError(request, CEC_ERR_BAD_JSON);
        else if (!requestObj.hasKey("key"))
            LSUtils::respondWithError(request, CEC_ERR_KEY_PARAM_MISSING);
        else
            LSUtils::respondWithError(request, CEC_ERR_SCHEMA_VALIDATION_FAILED);
        return true;
    } else {
        LSMessage *requestMessage = request.get();
        LSMessageRef(requestMessage);
        m_clients[++m_clientId] = requestMessage;
        handleGetConfig(requestObj);
        return true;
    }
}

void CECLunaService::handleGetConfig(pbnjson::JValue &requestObj) {

    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::GET_CONFIG, std::bind(&CECLunaService::callback, this, m_clientId, CommandType::GET_CONFIG,
                    std::placeholders::_1));

    std::shared_ptr<GetConfigReqData> data = std::make_shared<GetConfigReqData>();
    data->key = requestObj["key"].asString();
    command->setData(data);
    //Send command to CEC Controller
    //Create sample data and send the response
    std::shared_ptr<GetConfigResData> respData = std::make_shared<GetConfigResData>();
    respData->key = "vendor-id";
    respData->value = "LG";
    respData->returnValue = true;
    CECLunaService::callback(this, m_clientId, CommandType::GET_CONFIG, respData);
}

bool CECLunaService::setConfig(LSMessage &message) {
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema = STRICT_SCHEMA(PROPS_2(PROP(key, string), PROP(value, string)) REQUIRED_2(key, value));

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::setConfig code: " << parseError << "\n";
        if (JSON_PARSE_SCHEMA_ERROR != parseError)
            LSUtils::respondWithError(request, CEC_ERR_BAD_JSON);
        else if (!requestObj.hasKey("key"))
            LSUtils::respondWithError(request, CEC_ERR_KEY_PARAM_MISSING);
        else if (!requestObj.hasKey("value"))
            LSUtils::respondWithError(request, CEC_ERR_VALUE_PARAM_MISSING);
        else
            LSUtils::respondWithError(request, CEC_ERR_SCHEMA_VALIDATION_FAILED);
        return true;
    } else {
        LSMessage *requestMessage = request.get();
        LSMessageRef(requestMessage);
        m_clients[++m_clientId] = requestMessage;
        handleSetConfig(requestObj);
        return true;
    }
}

void CECLunaService::handleSetConfig(pbnjson::JValue &requestObj) {

    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::SET_CONFIG, std::bind(&CECLunaService::callback, this, m_clientId, CommandType::SET_CONFIG,
                    std::placeholders::_1));

    std::shared_ptr<SetConfigReqData> data = std::make_shared<SetConfigReqData>();

    data->key = requestObj["key"].asString();
    data->value = requestObj["value"].asString();
    command->setData(data);
    //Send command to CEC Controller
    //Create sample data and send the response
    std::shared_ptr<CommandResData> respData = std::make_shared<CommandResData>();
    respData->returnValue = true;
    CECLunaService::callback(this, m_clientId, CommandType::SET_CONFIG, respData); //Create sample data and send the response
}

void CECLunaService::callback(void *ctx, uint16_t clientId, enum CommandType type,
        std::shared_ptr<CommandResData> respData) {
    CECLunaService *pThis = static_cast<CECLunaService*>(ctx);

    if (!pThis)
        return;

    std::shared_ptr<ListAdaptersResData> data = std::static_pointer_cast < ListAdaptersResData > (respData);

    if (!data)
        return;

    if (pThis->m_clients.find(clientId) != pThis->m_clients.end()) {
        LSMessage *requestMessage = pThis->m_clients[clientId];
        LS::Message request(requestMessage);
        if (data->returnValue) {
            //get response object based on command type
            pbnjson::JValue responseObj = pbnjson::Object();
            responseObj.put("returnValue", true);
            pThis->parseResponseObject(responseObj, type, data);
            LSUtils::postToClient(request, responseObj);
        } else {
            if (data->error) {
                LSUtils::respondWithError(request, data->error->errorText, data->error->errorCode);
            } else {
                LSUtils::respondWithError(request, CEC_ERR_UNKNOWN_ERROR);
            }
        }
        pThis->m_clients.erase(clientId);
    }
}

void CECLunaService::parseResponseObject(pbnjson::JValue &responseObj, enum CommandType type,
        std::shared_ptr<CommandResData> respData) {
    switch (type) {
        case CommandType::LIST_ADAPTERS: {
            std::shared_ptr<ListAdaptersResData> data = std::static_pointer_cast < ListAdaptersResData > (respData);
            if (!data)
                return;
            pbnjson::JValue cecAdaptersArray = pbnjson::Array();
            for (auto const &cecDevice : data->cecAdapters) {
                cecAdaptersArray.append(cecDevice);
            }
            responseObj.put("cecAdapters", cecAdaptersArray);
            break;
        }
        case CommandType::SCAN: {
            std::shared_ptr<ScanResData> data = std::static_pointer_cast < ScanResData > (respData);

            if (!data)
                return;

            pbnjson::JValue devicesArray = pbnjson::Array();
            for (auto const &cecDevice : data->devices) {
                pbnjson::JValue device = pbnjson::Object();
                device.put("name", cecDevice.name);
                device.put("address", cecDevice.address);
                device.put("activeSource", cecDevice.activeSource);
                device.put("vendor", cecDevice.vendor);
                device.put("osdString", cecDevice.osdString);
                device.put("cecVersion", cecDevice.cecVersion);
                device.put("powerStatus", cecDevice.powerStatus);
                device.put("language", cecDevice.language);
                devicesArray.append(device);
            }
            responseObj.put("devices", devicesArray);
            notifyScanStatus(data);
            break;
        }
        case CommandType::SEND_COMMAND:
        case CommandType::SET_CONFIG: {
            responseObj.put("returnValue", true);
            break;
        }
        case CommandType::GET_CONFIG: {
            std::shared_ptr<GetConfigResData> data = std::static_pointer_cast < GetConfigResData > (respData);

            if (!data)
                return;
            responseObj.put("returnValue", true);
            responseObj.put("key", data->key);
            responseObj.put("value", data->value);
            break;
        }
    }
}

void CECLunaService::notifyScanStatus(std::shared_ptr<ScanResData> data) {

    pbnjson::JValue respObj = pbnjson::Object();
    respObj.put("returnValue", true);
    respObj.put("subscribed", true);
    pbnjson::JValue cecDevicesArray = pbnjson::Array();
    for (auto const &cecDevice : data->devices) {
        pbnjson::JValue device = pbnjson::Object();
        device.put("name", cecDevice.name);
        device.put("address", cecDevice.address);
        device.put("activeSource", cecDevice.activeSource);
        device.put("vendor", cecDevice.vendor);
        device.put("osdString", cecDevice.osdString);
        device.put("cecVersion", cecDevice.cecVersion);
        device.put("powerStatus", cecDevice.powerStatus);
        device.put("language", cecDevice.language);
        cecDevicesArray.append(device);
    }
    respObj.put("devices", cecDevicesArray);

    LSUtils::postToSubscriptionPoint(&mScanSubscriptions, respObj);
}
