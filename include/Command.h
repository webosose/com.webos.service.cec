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

enum CommandType {
    LIST_DEVICES, SCAN, SEND_COMMAND, GET_CONFIG, SET_CONFIG
};

typedef struct ErrorInfo {
    int errorCode;
    std::string errorText;
} ErrorInfo;

struct CommandReqData {
    virtual ~CommandReqData(){};
};

struct ListDevicesReqData: public CommandReqData {

};

struct ScanReqData: public CommandReqData {
    bool subscribed;
};

struct CecCommandArgs {
    std::string arg;
    std::string value;
};

struct CecCommand {
    std::string name;
    CecCommandArgs args;
};

struct SendCommandReqData: public CommandReqData {
    std::string device;
    std::string srcAddress;
    std::string destAddress;
    std::string timeout;
    CecCommand command;
};

struct GetConfigReqData: public CommandReqData {
    std::string confName;
};

struct SetConfigReqData: public CommandReqData {
    std::string confName;
    std::string confValue;
};

struct CommandResData {
    bool returnValue;
    std::shared_ptr<ErrorInfo> error;
    virtual ~CommandResData(){};
};

struct ListDevicesResData: public CommandResData {
    std::list<std::string> cecDevices;
};

struct CecDevice {
    std::string name;
    std::string address;
    std::string activeSource;
    std::string vendor;
    std::string osdString;
    std::string cecVersion;
    std::string powerStatus;
    std::string language;

    CecDevice(std::string nam, std::string addr, std::string activeSrc, std::string vdr, std::string osdStr,
            std::string cecVer, std::string powerStat, std::string lang) :
            name(nam), address(addr), activeSource(activeSrc), vendor(vdr), osdString(osdStr), cecVersion(cecVer), powerStatus(
                    powerStat), language(lang) {
    }
};

struct ScanResData: public CommandResData {
    std::list<CecDevice> devices;
};

struct GetConfigResData: public CommandResData {
    std::string confName;
    std::string confValue;
};

typedef std::function<void(std::shared_ptr<CommandResData>)> CommandCallback;

class Command {
public:
    Command(enum CommandType type, CommandCallback callback) :
            m_commandType(type), m_callback(callback) {
    }
    virtual ~Command(){};
    void setData(std::shared_ptr<CommandReqData> data) {
        m_data = data;
    }
    std::shared_ptr<CommandReqData> getData() {
        return m_data;
    }
    enum CommandType getType() {
        return m_commandType;
    }

private:
    enum CommandType m_commandType;
    CommandCallback m_callback;
    std::shared_ptr<CommandReqData> m_data;
};
