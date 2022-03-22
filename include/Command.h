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

#include <list>
#include <vector>
#include <memory>

#include "Logger.h"

const std::string DEFAULT_CEC_ADAPTER = "cec0";
const int DEFAULT_REPLY_TIMEOUT_MS = 1000;

enum CommandType {
    LIST_ADAPTERS, SCAN, SEND_COMMAND, GET_CONFIG, SET_CONFIG
};

typedef struct ErrorInfo {
    int errorCode;
    std::string errorText;
} ErrorInfo;

struct CommandReqData {
    virtual ~CommandReqData() {
    }
};

struct ListAdaptersReqData: public CommandReqData {

};

struct ScanReqData: public CommandReqData {
    std::string adapter = DEFAULT_CEC_ADAPTER;
};

struct CecCommandArg {
    std::string arg;
    std::string value;
};

struct CecCommand {
    std::string name;
    std::vector<CecCommandArg> args;
};

struct SendCommandReqData: public CommandReqData {
    std::string adapter = DEFAULT_CEC_ADAPTER;
    std::string destAddress;
    int32_t timeout = DEFAULT_REPLY_TIMEOUT_MS;
    CecCommand command;
};

struct GetConfigReqData: public CommandReqData {
    std::string key;
    std::string adapter = DEFAULT_CEC_ADAPTER;
};

struct SetConfigReqData: public CommandReqData {
    std::string key;
    std::string value;
    std::string adapter = DEFAULT_CEC_ADAPTER;
};

struct CommandResData {
    bool returnValue;
    std::shared_ptr<ErrorInfo> error;
    virtual ~CommandResData() {
    }
};

struct ListAdaptersResData: public CommandResData {
    std::list<std::string> cecAdapters;
};

class CecDevice {
    std::string m_name;
    std::string m_address;
    std::string m_activeSource;
    std::string m_vendor;
    std::string m_osd;
    std::string m_cecVersion;
    std::string m_powerStatus;
    std::string m_language;

public:
    CecDevice(std::string name, std::string addr, std::string activeSrc, std::string vdr, std::string osd,
            std::string cecVer, std::string powerStat, std::string lang) :
            m_name(name), m_address(addr), m_activeSource(activeSrc), m_vendor(vdr), m_osd(osd), m_cecVersion(cecVer), m_powerStatus(
                    powerStat), m_language(lang) {
    }

    bool hasLogicalAddress() {
        return m_address.find_first_not_of("0123456789") == std::string::npos;
    }

    const std::string& getName() const {
        return m_name;
    }

    const std::string& getAddress() const {
        return m_address;
    }

    const std::string& getActiveSource() const {
        return m_activeSource;
    }

    const std::string& getVendor() const {
        return m_vendor;
    }

    const std::string& getOsd() const {
        return m_osd;
    }

    const std::string& getCecVersion() const {
        return m_cecVersion;
    }

    const std::string& getPowerStatus() const {
        return m_powerStatus;
    }

    const std::string& getLanguage() const {
        return m_language;
    }

    void printDeviceInfo() const {
        AppLogDebug() <<"CecDevice Info:\n";
        AppLogDebug() <<"Name: " << m_name << "\n";
        AppLogDebug() <<"Address: " << m_address << "\n";
        AppLogDebug() <<"ActiveSource: " << m_activeSource << "\n";
        AppLogDebug() <<"Vendor: " << m_vendor << "\n";
        AppLogDebug() <<"OSD: " << m_osd << "\n";
        AppLogDebug() <<"Cec Version: " << m_cecVersion << "\n";
        AppLogDebug() <<"Power Status: " << m_powerStatus << "\n";
        AppLogDebug() <<"Language: " << m_language << "\n";
    }
};

struct ScanResData: public CommandResData {
    std::list<CecDevice> devices;
};

struct SendCommandPayload {
    std::string key;
    std::string value;
};

struct SendCommandResData: public CommandResData {
    std::vector<SendCommandPayload> payload;
};

struct GetConfigResData: public CommandResData {
    std::string key;
    std::string value;
};

typedef std::function<void(std::shared_ptr<CommandResData>)> CommandCallback;

class Command {
public:
    Command(enum CommandType type, CommandCallback callback) :
            m_commandType(type), m_callback(callback) {
    }
    virtual ~Command() {
    }
    void setData(std::shared_ptr<CommandReqData> data) {
        m_data = data;
    }
    std::shared_ptr<CommandReqData> getData() {
        return m_data;
    }
    enum CommandType getType() {
        return m_commandType;
    }

    CommandCallback getCallback() {
        return m_callback;
    }

private:
    enum CommandType m_commandType;
    CommandCallback m_callback;
    std::shared_ptr<CommandReqData> m_data;
};
