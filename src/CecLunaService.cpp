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

#include "CecErrors.h"
#include "Ls2Utils.h"
#include "CecController.h"
#include "CecLunaService.h"

const std::string SERVICE_NAME = "com.webos.service.cec";

CecLunaService::CecLunaService() :
        LS::Handle(SERVICE_NAME.c_str()) {
    registerMethods();
    AppLogInfo()<<" CecLunaService:: call async method"<<"\n";
    CecController::getInstance()->m_InitFut = std::async(std::launch::async, []() {
        return CecController::getInstance()->initialize();
        });
}

CecLunaService::~CecLunaService() {

}

void CecLunaService::registerMethods() {
    AppLogDebug() <<__func__<<"\n";
    //Register Luna Methods for CeC service
    LS_CREATE_CATEGORY_BEGIN(CecLunaService, base) LS_CATEGORY_CLASS_METHOD(CecLunaService, listAdapters)
    LS_CATEGORY_CLASS_METHOD(CecLunaService, scan)
    LS_CATEGORY_CLASS_METHOD(CecLunaService, sendCommand)
    LS_CATEGORY_CLASS_METHOD(CecLunaService, getConfig)
    LS_CATEGORY_CLASS_METHOD(CecLunaService, setConfig)
    LS_CREATE_CATEGORY_END

    registerCategory("/", LS_CATEGORY_TABLE_NAME(base), NULL, NULL);
    setCategoryData("/", this);
}

bool CecLunaService::listAdapters(LSMessage &message) {

    AppLogDebug() <<__func__<<"\n";
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

void CecLunaService::handleListAdapters(pbnjson::JValue &requestObj) {

    AppLogDebug() <<__func__<<"\n";
    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::LIST_ADAPTERS, std::bind(&CecLunaService::callback, this, m_clientId,
                    CommandType::LIST_ADAPTERS, std::placeholders::_1));
    //Send command to CEC Controller
    CecController::getInstance()->HandleCommand(command);
}

bool CecLunaService::scan(LSMessage &message) {

    AppLogDebug() <<__func__<<"\n";
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema = SCHEMA_1(PROP(adapter, string));

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
        return true;
    }
}

void CecLunaService::handleScan(pbnjson::JValue &requestObj) {

    AppLogDebug() <<__func__<<"\n";
    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::SCAN, std::bind(&CecLunaService::callback, this, m_clientId, CommandType::SCAN,
                    std::placeholders::_1));

    std::shared_ptr<ScanReqData> data = std::make_shared<ScanReqData>();

    if (requestObj.hasKey("adapter")) {
        data->adapter = requestObj["adapter"].asString();
    }
    command->setData(data);
    //Send command to CEC Controller
    CecController::getInstance()->HandleCommand(command);
}

bool CecLunaService::sendCommand(LSMessage &message) {

    AppLogDebug() <<__func__<<"\n";
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema =
            STRICT_SCHEMA(
                    PROPS_4(PROP(adapter, string),
                            PROP(destAddress, string),
                            PROP(timeout, integer),
                            OBJECT(command, OBJSCHEMA_2_STRICT(
                                    PROP(name, string),
                                    OBJARRAY(args, OBJSCHEMA_2_STRICT(
                                            PROP(arg, string),
                                            PROP(value, string),
                                            REQUIRED_1(arg))),
                                    REQUIRED_2(name, args))))
                    REQUIRED_2(destAddress, command));

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::sendCommand code: " << parseError << "\n";
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
        handleSendCommand(requestObj);
        return true;
    }
}

void CecLunaService::handleSendCommand(pbnjson::JValue &requestObj) {

    AppLogDebug() <<__func__<<"\n";
    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::SEND_COMMAND, std::bind(&CecLunaService::callback, this, m_clientId,
                    CommandType::SEND_COMMAND, std::placeholders::_1));

    std::shared_ptr<SendCommandReqData> data = std::make_shared<SendCommandReqData>();
    if (requestObj.hasKey("adapter")) {
        data->adapter = requestObj["adapter"].asString();
    }

    data->destAddress = requestObj["destAddress"].asString();
    if (requestObj.hasKey("timeout")) {
        data->timeout = requestObj["timeout"].asNumber<int32_t>();
    }

    CecCommand ceccommand;
    auto cecCommandObj = requestObj["command"];
    ceccommand.name = cecCommandObj["name"].asString();
    if (cecCommandObj.hasKey("args")) {
        auto argsObj = cecCommandObj["args"];
        ssize_t argsSize = argsObj.arraySize();

        for (auto i = 0; i < argsSize; ++i) {
            auto argObj = argsObj[i];
            CecCommandArg commandArg;
            commandArg.arg = argObj["arg"].asString();
            if (argObj.hasKey("value")) {
                commandArg.value = argObj["value"].asString();
            }
            ceccommand.args.push_back(commandArg);
        }
    }
    data->command = ceccommand;

    command->setData(data);
    //Send command to CEC Controller
    CecController::getInstance()->HandleCommand(command);
}

bool CecLunaService::getConfig(LSMessage &message) {

    AppLogDebug() <<__func__<<"\n";
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema = STRICT_SCHEMA(PROPS_2(PROP(key, string), PROP(adapter, string)) REQUIRED_1(key));

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::getConfig code: " << parseError << "\n";
        if (JSON_PARSE_SCHEMA_ERROR != parseError)
            LSUtils::respondWithError(request, CEC_ERR_BAD_JSON);
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

void CecLunaService::handleGetConfig(pbnjson::JValue &requestObj) {

    AppLogDebug() <<__func__<<"\n";
    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::GET_CONFIG, std::bind(&CecLunaService::callback, this, m_clientId, CommandType::GET_CONFIG,
                    std::placeholders::_1));

    std::shared_ptr<GetConfigReqData> data = std::make_shared<GetConfigReqData>();
    data->key = requestObj["key"].asString();
    if (requestObj.hasKey("adapter")) {
        data->adapter = requestObj["adapter"].asString();
    }
    command->setData(data);
    //Send command to CEC Controller
    CecController::getInstance()->HandleCommand(command);
}

bool CecLunaService::setConfig(LSMessage &message) {

    AppLogDebug() <<__func__<<"\n";
    LS::Message request(&message);
    pbnjson::JValue requestObj;
    const std::string schema = STRICT_SCHEMA(
            PROPS_3(PROP(key, string), PROP(value, string), PROP(adapter, string)) REQUIRED_2(key, value));

    int parseError = 0;
    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError)) {
        AppLogError() << "Parser error: CecLunaService::setConfig code: " << parseError << "\n";
        if (JSON_PARSE_SCHEMA_ERROR != parseError)
            LSUtils::respondWithError(request, CEC_ERR_BAD_JSON);
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

void CecLunaService::handleSetConfig(pbnjson::JValue &requestObj) {

    AppLogDebug() <<__func__<<"\n";
    std::shared_ptr<Command> command = std::make_shared < Command
            > (CommandType::SET_CONFIG, std::bind(&CecLunaService::callback, this, m_clientId, CommandType::SET_CONFIG,
                    std::placeholders::_1));

    std::shared_ptr<SetConfigReqData> data = std::make_shared<SetConfigReqData>();

    data->key = requestObj["key"].asString();
    data->value = requestObj["value"].asString();
    if (requestObj.hasKey("adapter")) {
        data->adapter = requestObj["adapter"].asString();
    }
    command->setData(data);
    //Send command to CEC Controller
    CecController::getInstance()->HandleCommand(command);
}

void CecLunaService::callback(void *ctx, uint16_t clientId, enum CommandType type,
        std::shared_ptr<CommandResData> respData) {

    AppLogDebug() <<__func__<<"\n";
    CecLunaService *pThis = static_cast<CecLunaService*>(ctx);

    if (!pThis)
        return;

    if (pThis->m_clients.find(clientId) != pThis->m_clients.end()) {
        LSMessage *requestMessage = pThis->m_clients[clientId];
        LS::Message request(requestMessage);
        if (respData->returnValue) {
            //get response object based on command type
            pbnjson::JValue responseObj = pbnjson::Object();
            responseObj.put("returnValue", true);
            pThis->parseResponseObject(responseObj, type, respData);
            LSUtils::postToClient(request, responseObj);
        } else {
            if (respData->error) {
                LSUtils::respondWithError(request, respData->error->errorText, respData->error->errorCode);
            } else {
                LSUtils::respondWithError(request, CEC_ERR_UNKNOWN_ERROR);
            }
        }
        pThis->m_clients.erase(clientId);
    }
}

void CecLunaService::parseResponseObject(pbnjson::JValue &responseObj, enum CommandType type,
        std::shared_ptr<CommandResData> respData) {

    AppLogDebug() <<__func__<<"\n";
    switch (type) {
        case CommandType::LIST_ADAPTERS: {
            AppLogDebug() <<__func__<<" parse listadapters response\n";
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
            AppLogDebug() <<__func__<<" parse scan response\n";
            std::shared_ptr<ScanResData> data = std::static_pointer_cast < ScanResData > (respData);

            if (!data)
                return;

            pbnjson::JValue devicesArray = pbnjson::Array();
            for (auto const &cecDevice : data->devices) {
                pbnjson::JValue device = pbnjson::Object();
                device.put("name", cecDevice.getName());
                device.put("address", cecDevice.getAddress());
                device.put("activeSource", cecDevice.getActiveSource());
                device.put("vendor", cecDevice.getVendor());
                device.put("osd", cecDevice.getOsd());
                device.put("cecVersion", cecDevice.getCecVersion());
                device.put("powerStatus", cecDevice.getPowerStatus());
                device.put("language", cecDevice.getLanguage());
                devicesArray.append(device);
            }
            responseObj.put("devices", devicesArray);
            break;
        }
        case CommandType::SEND_COMMAND: {
            AppLogDebug() <<__func__<<" parse send command response\n";
            std::shared_ptr<SendCommandResData> data = std::static_pointer_cast < SendCommandResData > (respData);

            if (data && data->payload.size()) {
                pbnjson::JValue responseArray = pbnjson::Array();
                for (auto &element : data->payload) {
                    pbnjson::JValue response = pbnjson::Object();
                    response.put(element.key, element.value);
                    responseArray.append(response);
                }
                responseObj.put("payload", responseArray);
            }
            break;
        }
        case CommandType::SET_CONFIG: {
            AppLogDebug() <<__func__<<" parse setconfig response\n";
            //Nothing to handle
            break;
        }
        case CommandType::GET_CONFIG: {
            AppLogDebug() <<__func__<<" parse getconfig response\n";
            std::shared_ptr<GetConfigResData> data = std::static_pointer_cast < GetConfigResData > (respData);

            if (!data)
                return;
            responseObj.put("key", data->key);
            responseObj.put("value", data->value);
            break;
        }
    }
}
