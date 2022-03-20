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

#include <cstring>
#include "DefaultCecHandler.h"

bool DefaultCecHandler::mIsObjRegistered = DefaultCecHandler::RegisterObject();

std::list<std::shared_ptr<Command>> DefaultCecHandler::mCmdList;
std::mutex DefaultCecHandler::mMutex;
std::list<CecDevice> DefaultCecHandler::mDeviceInfoList;
std::list<std::string> DefaultCecHandler::mAdaptersList;

static void printResp(std::vector<std::string> resp) {
  for (auto it=resp.begin(); it!=resp.end(); ++it) {
    AppLogDebug()<<*it;
  }
}

DefaultCecHandler::DefaultCecHandler() :
                       CecHandler() {

  mQueue.setCallback(DefaultCecHandler::HandleMessageCb);

  std::shared_ptr<Command> listAdapterCommand = std::make_shared<Command>(CommandType::LIST_ADAPTERS,
                                                                          [](std::shared_ptr<CommandResData> resp) -> void {});
  std::unique_lock < std::mutex > lock(mMutex);
  mCmdList.push_back(listAdapterCommand);
  lock.unlock();

  std::shared_ptr<MessageData> msgDataAdapter = std::make_shared<MessageData>();
  msgDataAdapter->type = LIST_ADAPTERS;
  mQueue.addMessage(msgDataAdapter);
}

DefaultCecHandler::~DefaultCecHandler() {
}

void DefaultCecHandler::HandleMessageCb(CommandType type, std::vector<std::string> resp) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  switch(type) {
    case SEND_COMMAND:
      return HandleSendCommandCb(resp);

    case LIST_ADAPTERS:
      return HandleListAdaptersCb(resp);

    case SCAN:
      return HandleScanCb(resp);

    case GET_CONFIG:
      return HandleGetConfigCb(resp);

    case SET_CONFIG:
      return HandleSetConfigCb(resp);

    default:
      AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Invalid command type";
      return;
  }
}

std::string DefaultCecHandler::GetValue(std::string str) {
  std::size_t pos = str.find_first_of(':');
  if (pos == std::string::npos  || pos == str.size())
    return "";
  std::size_t actualPos = str.find_first_not_of(' ', pos + 1);
  if (actualPos == std::string::npos)
    return "";
  return str.substr(actualPos);
}

void DefaultCecHandler::HandleSystemInfoResp(std::shared_ptr<SendCommandReqData> commandData, std::vector<std::string> resp, std::shared_ptr<SendCommandResData> respCmd) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  for (auto it = commandData->command.args.begin();  it!=commandData->command.args.end(); ++it) {
    SendCommandPayload payload;
    payload.key = (*it).arg;
    for (auto itr=resp.begin(); itr!=resp.end(); ++itr) {

      if (payload.key=="vendor-id") {
        if ((*itr).find("vendor id") != std::string::npos) {
          payload.value = GetValue(*itr);
          break;
        }
        continue;
      }

      if (payload.key=="version") {
        if ((*itr).find("CEC version") != std::string::npos) {
          payload.value = (*itr).substr(std::strlen("CEC version "));
          break;
        }
        continue;
      }

      if (payload.key=="name") {
        if ((*itr).find("OSD name of device") != std::string::npos) {
          std::size_t startPos = ((*itr).find_first_of('\'') == std::string::npos)?0:((*itr).find_first_of('\'') + 1);
          std::size_t endPos = ((*itr).substr(startPos).find_last_of('\'') == std::string::npos)?(*itr).size()-1:(*itr).find_last_of('\'');
          AppLogDebug()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" startPos = "<<startPos<<" endPos = "<<endPos<<" size = "<<(*itr).size();
          payload.value = (*itr).substr(startPos, endPos-startPos);
          break;
        }
        continue;
      }

      if (payload.key=="language") {
  	    if ((*itr).find("language") != std::string::npos) {
          std::size_t startPos = ((*itr).find_first_of('\'') == std::string::npos)?0:((*itr).find_first_of('\'') + 1);
          std::size_t endPos = ((*itr).substr(startPos).find_last_of('\'') == std::string::npos)?(*itr).size()-1:(*itr).find_last_of('\'');
          AppLogDebug()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" startPos = "<<startPos<<" endPos = "<<endPos<<" size = "<<(*itr).size();
          payload.value = (*itr).substr(startPos, endPos-startPos);
          break;
        }
        continue;
      }

      if (payload.key=="is-active") {
  	    if ((*itr).find("active") != std::string::npos) {
  	      if ((*itr).find("not active") != std::string::npos) {
            payload.value = "false";
          } else {
            payload.value = "true";
          }
          break;
        }
        continue;
      }
    }
    respCmd->payload.push_back(payload);
  }
}

void DefaultCecHandler::HandleSendCommandCb(std::vector<std::string> resp) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  AppLogDebug()<<"SEND_COMMAND Response : START";
  printResp(resp);
  AppLogDebug()<<"SEND_COMMAND Response : END";

  std::unique_lock < std::mutex > lock(mMutex);
  std::shared_ptr<Command> command = mCmdList.front();
  if (command->getType() != SEND_COMMAND)
    return;
  mCmdList.pop_front();
  lock.unlock();

  std::shared_ptr<SendCommandResData> respCmd = std::make_shared<SendCommandResData>();
  CommandCallback callback = command->getCallback();
  respCmd->returnValue = true;

  if (!resp.size()) {
    AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Empty response reveived";
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  std::shared_ptr<SendCommandReqData> commandData = std::static_pointer_cast<SendCommandReqData>(command->getData());

  if (commandData->command.name=="report-power-status") {
    SendCommandPayload payload;
    payload.key = commandData->command.args.front().arg;
    if (commandData->command.args.front().value.empty()) {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("power status") != std::string::npos) {
          payload.value = GetValue(*it);
          respCmd->payload.push_back(payload);
          break;
        }
      }
    } else {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("response") != std::string::npos) {
          if (GetValue(*it) != "success")
            AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" error: "<<GetValue(*it);
          break;
        }
      }
    }
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  if (commandData->command.name=="report-audio-status") {
    SendCommandPayload payload;
    payload.key = commandData->command.args.front().arg;
    if (commandData->command.args.front().value.empty()) {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("mute") != std::string::npos || (*it).find("volume") != std::string::npos) {
          payload.value = GetValue(*it);
          respCmd->payload.push_back(payload);
          break;
        }
      }
    } else {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("mute") != std::string::npos || (*it).find("volume") != std::string::npos) {
          if (GetValue(*it) != "7F")
            AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" error: "<<GetValue(*it);
          break;
        }
      }
    }
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  if (commandData->command.name=="set-volume") {
    SendCommandPayload payload;
    payload.key = commandData->command.args.front().arg;
    if (commandData->command.args.front().value.empty()) {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("volume") != std::string::npos) {
          payload.value = GetValue(*it);
          respCmd->payload.push_back(payload);
          break;
        }
      }
    } else {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("volume") != std::string::npos) {
          if (GetValue(*it) != "7F")
            AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" error: "<<GetValue(*it);
          break;
        }
      }
    }
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  if (commandData->command.name=="osd-display") {
    SendCommandPayload payload;
    payload.key = commandData->command.args.front().arg;
    if (commandData->command.args.front().value.empty()) {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("osd string") != std::string::npos) {
          payload.value = GetValue(*it);
          respCmd->payload.push_back(payload);
          break;
        }
      }
    } else {
      for (auto it=resp.begin(); it!=resp.end(); ++it) {
        if ((*it).find("response") != std::string::npos) {
          if (GetValue(*it) != "success")
            AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" error: "<<GetValue(*it);
          break;
        }
      }
    }
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  if (commandData->command.name=="active") {
    SendCommandPayload payload;
    payload.key = commandData->command.args.front().arg;
    for (auto it=resp.begin(); it!=resp.end(); ++it) {
      if ((*it).find("response") != std::string::npos) {
        if (GetValue(*it) != "success") {
          payload.value = GetValue(*it);
          respCmd->payload.push_back(payload);
        }
        break;
      }
    }
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  if (commandData->command.name=="one-touch-play") {
    SendCommandPayload payload;
    payload.key = commandData->command.args.front().arg;
    for (auto it=resp.begin(); it!=resp.end(); ++it) {
      if ((*it).find("response") != std::string::npos) {
        if (GetValue(*it) != "success") {
          payload.value = GetValue(*it);
          respCmd->payload.push_back(payload);
        }
        break;
      }
    }
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  if (commandData->command.name=="system-information") {
    HandleSystemInfoResp(commandData, resp, respCmd);
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  if (commandData->command.name=="vendor-commands") {
    for (auto it=resp.begin(); it!=resp.end(); ++it) {
      if ((*it).find("waiting for input") != std::string::npos) {
        break;
      }
    }
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }
}

void DefaultCecHandler::HandleScanCb(std::vector<std::string> resp) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  AppLogDebug()<<"SCAN_COMMAND Response : START";
  printResp(resp);
  AppLogDebug()<<"SCAN_COMMAND Response : END";

  std::unique_lock < std::mutex > lock(mMutex);
  std::shared_ptr<Command> command = mCmdList.front();
  if (command->getType() != SCAN)
    return;
  mCmdList.pop_front();
  lock.unlock();

  std::shared_ptr<ScanResData> respCmd = std::make_shared<ScanResData>();
  CommandCallback callback = command->getCallback();
  respCmd->returnValue = true;

  if (!resp.size()) {
    AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Empty response reveived";
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  for (auto it=resp.begin(); it!=resp.end(); ) {
    if ((*it).find("device") == std::string::npos) {
      ++it;
      continue;
    }

    std::string name = GetValue(*it);
    ++it;

    std::string address;
    std::string activeSource;
    std::string vendor;
    std::string osd;
    std::string cecVersion;
    std::string powerStatus;
    std::string language;

    for ( ;it!=resp.end(); ++it) {
      if ((*it).find("device") != std::string::npos) {
        break;
      }

      if (address.empty() && (*it).find("address") != std::string::npos) {
        address = GetValue(*it);
        continue;
      }
      if (activeSource.empty() && (*it).find("active source") != std::string::npos) {
        activeSource = GetValue(*it);
        continue;
      }
      if (vendor.empty() && (*it).find("vendor") != std::string::npos) {
        vendor = GetValue(*it);
        continue;
      }
      if (osd.empty() && (*it).find("osd string") != std::string::npos) {
        osd = GetValue(*it);
        continue;
      }
      if (cecVersion.empty() && (*it).find("CEC version") != std::string::npos) {
        cecVersion = GetValue(*it);
        continue;
      }
      if (powerStatus.empty() && (*it).find("power status") != std::string::npos) {
        powerStatus = GetValue(*it);
        continue;
      }
      if (language.empty() && (*it).find("language") != std::string::npos) {
        language = GetValue(*it);
        continue;
      }
    }

    CecDevice dev {name,
                  address,
                  activeSource,
                  vendor,
                  osd,
                  cecVersion,
                  powerStatus,
                  language};

    respCmd->devices.push_back(dev);

    std::unique_lock < std::mutex > lock(mMutex);
    for (auto itr=mDeviceInfoList.begin(); itr!=mDeviceInfoList.end(); ++itr) {
      if ((*itr).getAddress() == address) {
        mDeviceInfoList.erase(itr);
        break;
      }
    }
    mDeviceInfoList.push_back(dev);
    lock.unlock();
  }

  callback(std::static_pointer_cast<CommandResData>(respCmd));
}

void DefaultCecHandler::HandleListAdaptersCb(std::vector<std::string> resp) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  AppLogDebug()<<"LISTADAPTERS_COMMAND Response : START";
  printResp(resp);
  AppLogDebug()<<"LISTADAPTERS_COMMAND Response : END";

  std::unique_lock < std::mutex > lock(mMutex);
  std::shared_ptr<Command> command = mCmdList.front();
  if (command->getType() != LIST_ADAPTERS)
    return;
  mCmdList.pop_front();
  lock.unlock();

  std::shared_ptr<ListAdaptersResData> respCmd = std::make_shared<ListAdaptersResData>();
  CommandCallback callback = command->getCallback();
  respCmd->returnValue = true;

  if (!resp.size()) {
    AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Empty response reveived";
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  lock.lock();
  mAdaptersList.clear();
  lock.unlock();

  for (auto it=resp.begin(); it!=resp.end(); ) {
    if ((*it).find("com port") == std::string::npos) {
      ++it;
      continue;
    }
    if (GetValue(*it) == "RPI") {
      respCmd->cecAdapters.push_back("cec0");
      lock.lock();
      mAdaptersList.push_back("cec0");
      lock.unlock();
    } else {
      respCmd->cecAdapters.push_back(GetValue(*it));
      lock.lock();
      mAdaptersList.push_back(GetValue(*it));
      lock.unlock();
    }
    ++it;
  }
  callback(std::static_pointer_cast<CommandResData>(respCmd));
}

void DefaultCecHandler::HandleGetConfigCb(std::vector<std::string> resp) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  AppLogDebug()<<"GETCONFIG_COMMAND Response : START";
  printResp(resp);
  AppLogDebug()<<"GETCONFIG_COMMAND Response : END";

  std::unique_lock < std::mutex > lock(mMutex);
  std::shared_ptr<Command> command = mCmdList.front();
  if (command->getType() != GET_CONFIG)
    return;
  mCmdList.pop_front();
  lock.unlock();

  std::shared_ptr<GetConfigResData> respCmd = std::make_shared<GetConfigResData>();
  CommandCallback callback = command->getCallback();
  respCmd->returnValue = true;

  if (!resp.size()) {
    AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Empty response reveived";
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return;
  }

  std::shared_ptr<GetConfigReqData> configData = std::static_pointer_cast<GetConfigReqData>(command->getData());
  respCmd->key = configData->key;
  for (auto it=resp.begin(); it!=resp.end(); ++it) {
    if (configData->key=="vendorId") {
      if ((*it).find("vendor id") != std::string::npos) {
        respCmd->value = GetValue(*it);
        break;
      }
      continue;
    }

    if (configData->key=="version") {
      if ((*it).find("CEC version") != std::string::npos) {
        respCmd->value = GetValue(*it);
        break;
      }
      continue;
    }

    if (configData->key=="osd") {
      if ((*it).find("osd string") != std::string::npos) {
        respCmd->value = GetValue(*it);
        break;
      }
      continue;
    }

    if (configData->key=="language") {
      if ((*it).find("language") != std::string::npos) {
        respCmd->value = GetValue(*it);
        break;
      }
      continue;
    }

    if (configData->key=="powerState") {
      if ((*it).find("power status") != std::string::npos) {
        respCmd->value = GetValue(*it);
        break;
      }
      continue;
    }

    if (configData->key=="physicalAddress") {
      if ((*it).find("address") != std::string::npos && (*it).find("logical address") == std::string::npos) {
        respCmd->value = GetValue(*it);
        break;
      }
      continue;
    }

    if (configData->key=="logicalAddress") {
      if ((*it).find("logical address") != std::string::npos) {
        respCmd->value = GetValue(*it);
        respCmd->value = (*it).substr(std::strlen("logical address "));
        break;
      }
      continue;
    }
  }
  callback(std::static_pointer_cast<CommandResData>(respCmd));
}

void DefaultCecHandler::HandleSetConfigCb(std::vector<std::string> resp) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  AppLogDebug()<<"SETCONFIG_COMMAND Response : START";
  printResp(resp);
  AppLogDebug()<<"SETCONFIG_COMMAND Response : END";

  std::unique_lock < std::mutex > lock(mMutex);
  std::shared_ptr<Command> command = mCmdList.front();
  if (command->getType() != SET_CONFIG)
    return;
  mCmdList.pop_front();
  lock.unlock();

  std::shared_ptr<CommandResData> respCmd = std::make_shared<CommandResData>();
  CommandCallback callback = command->getCallback();
  respCmd->returnValue = true;

  if (!resp.size()) {
    AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Empty response reveived";
    callback(respCmd);
    return;
  }

  //TODO: Check error and set error values.

  callback(respCmd);
}

bool DefaultCecHandler::HandleCommand(std::shared_ptr<Command> command) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;

  ErrorInfo errInfo;
  bool errorFound = false;
  switch(ValidateCommand(command)) {
    case HANDLER_ERROR_INVALID_PARAMTERS:
      errInfo.errorCode=2;
      errInfo.errorText="Invalid input parameter";
      errorFound = true;
    break;
    case HANDLER_ERROR_INVALID_COMMAND:
      errInfo.errorCode=5;
      errInfo.errorText="Invalid input command";
      errorFound = true;
    break;
    case HANDLER_ERROR_INVALID_ADAPTER:
      errInfo.errorCode=6;
      errInfo.errorText="CEC adapter doesn't exist";
      errorFound = true;
    break;
    case HANDLER_ERROR_INVALID_DESTINATION:
      errInfo.errorCode=4;
      errInfo.errorText="Destination device not found";
      errorFound = true;
    break;

    default:
    break;
  }

  if (errorFound) {

    CommandCallback callback = command->getCallback();
    std::shared_ptr<CommandResData> respCmd;

    switch(command->getType()) {
      case SEND_COMMAND:
        respCmd = std::static_pointer_cast<CommandResData>(std::make_shared<SendCommandResData>());
      break;

      case LIST_ADAPTERS:
        respCmd = std::static_pointer_cast<CommandResData>(std::make_shared<ListAdaptersResData>());
      break;

      case SCAN:
        respCmd = std::static_pointer_cast<CommandResData>(std::make_shared<ScanResData>());
      break;

      case GET_CONFIG:
        respCmd = std::static_pointer_cast<CommandResData>(std::make_shared<GetConfigResData>());
      break;

      case SET_CONFIG:
        respCmd = std::make_shared<CommandResData>();
      break;

      default:
        AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Invalid command type";
        respCmd = std::make_shared<CommandResData>();
      break;
    }

    respCmd->returnValue = false;
    respCmd->error = std::make_shared<ErrorInfo>(ErrorInfo{errInfo.errorCode, errInfo.errorText});
    callback(std::static_pointer_cast<CommandResData>(respCmd));
    return true;
  }

  std::unique_lock < std::mutex > lock(mMutex);
  mCmdList.push_back(command);
  lock.unlock();
  switch(command->getType()) {
    case SEND_COMMAND:
      return HandleSendCommand(command);

    case LIST_ADAPTERS:
      return HandleListAdapters(command);

    case SCAN:
      return HandleScan(command);

    case GET_CONFIG:
      return HandleGetConfig(command);

    case SET_CONFIG:
      return HandleSetConfig(command);

    default:
     AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Invalid command type";
      return false;
  }
}

std::shared_ptr<CecDevice> DefaultCecHandler::GetDeviceInfo(std::string destAddress) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  std::unique_lock < std::mutex > lock(mMutex);
  for (auto it=mDeviceInfoList.begin(); it!=mDeviceInfoList.end(); ++it) {
    if ((*it).getAddress() == destAddress) {
      std::shared_ptr<CecDevice> retVal = std::make_shared<CecDevice>(*it);
      return retVal;
    }
  }
  return std::shared_ptr<CecDevice>();
}

bool DefaultCecHandler::HandleSendCommand(std::shared_ptr<Command> command) {
  std::shared_ptr<MessageData> msgData = std::make_shared<MessageData>();
  std::shared_ptr<SendCommandReqData> commandData = std::static_pointer_cast<SendCommandReqData>(command->getData());

  msgData->type = SEND_COMMAND;

  if (!commandData->adapter.empty())
    msgData->params["adapter"] = commandData->adapter;
  msgData->params["destAddress"] = commandData->destAddress;
  msgData->params["timeout"] = std::to_string(commandData->timeout);
  msgData->params["cmd-name"] = commandData->command.name;

  for (auto it = commandData->command.args.begin();  it!=commandData->command.args.end(); ++it) {

    AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" arg: "<<(*it).arg;
 
    if (!((*it).value.empty())) {
      msgData->params[(*it).arg] = (*it).value;
      AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" arg: "<<(*it).arg<<" value: "<<(*it).value;
    } else {
      AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" arg: "<<(*it).arg;
      msgData->params[(*it).arg];
    }
  }
  mQueue.addMessage(msgData);

  return true;
}

bool DefaultCecHandler::HandleScan(std::shared_ptr<Command> command) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  std::shared_ptr<MessageData> msgData = std::make_shared<MessageData>();
  std::shared_ptr<ScanReqData> scanData = std::static_pointer_cast<ScanReqData>(command->getData());

  msgData->type = SCAN;
  if (!scanData->adapter.empty())
    msgData->params["adapter"] = scanData->adapter;

  mQueue.addMessage(msgData);

  return true;
}

bool DefaultCecHandler::HandleListAdapters(std::shared_ptr<Command> command) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  std::shared_ptr<MessageData> msgData = std::make_shared<MessageData>();
  std::shared_ptr<ListAdaptersReqData> adapterData = std::static_pointer_cast<ListAdaptersReqData>(command->getData());

  msgData->type = LIST_ADAPTERS;

  mQueue.addMessage(msgData);

  return true;
}

bool DefaultCecHandler::HandleGetConfig(std::shared_ptr<Command> command) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  std::shared_ptr<MessageData> msgData = std::make_shared<MessageData>();
  std::shared_ptr<GetConfigReqData> configData = std::static_pointer_cast<GetConfigReqData>(command->getData());

  msgData->type = GET_CONFIG;

  msgData->params[configData->key];
  if (!configData->adapter.empty())
    msgData->params["adapter"] = configData->adapter;

  mQueue.addMessage(msgData);

  return true;
}

bool DefaultCecHandler::HandleSetConfig(std::shared_ptr<Command> command) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  std::shared_ptr<MessageData> msgData = std::make_shared<MessageData>();
  std::shared_ptr<SetConfigReqData> configData = std::static_pointer_cast<SetConfigReqData>(command->getData());

  msgData->type = SET_CONFIG;

  msgData->params[configData->key] = configData->value;
  if (!configData->adapter.empty())
    msgData->params["adapter"] = configData->adapter;

  mQueue.addMessage(msgData);

  return true;
}

HandlerErrorCode DefaultCecHandler::ValidateCommand(std::shared_ptr<Command> command) {
  AppLogInfo()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__;
  switch(command->getType()) {
    case SEND_COMMAND:
      return ValidateSendCommand(command);

    case LIST_ADAPTERS:
      return ValidateListAdapters(command);

    case SCAN:
      return ValidateScan(command);

    case GET_CONFIG:
      return ValidateGetConfig(command);

    case SET_CONFIG:
      return ValidateSetConfig(command);

    default:
     AppLogError()<<" DefaultCecHandler::"<<__func__<<":"<<__LINE__<<" Invalid command type";
      return HANDLER_ERROR_INVALID_COMMAND;
  }
}

HandlerErrorCode DefaultCecHandler::ValidateAdapter(std::string adapter) {
  std::unique_lock < std::mutex > lock(mMutex);
  for (auto it = mAdaptersList.begin();  it!=mAdaptersList.end(); ++it) {
    if ((*it) == adapter)
      return HANDLER_ERROR_OK;
  }
  return HANDLER_ERROR_INVALID_ADAPTER;
}

HandlerErrorCode DefaultCecHandler::ValidateAddress(std::string address) {
  std::unique_lock < std::mutex > lock(mMutex);
  for (auto it = mDeviceInfoList.begin();  it!=mDeviceInfoList.end(); ++it) {
    if ((*it).getAddress() == address)
      return HANDLER_ERROR_OK;
  }
  return HANDLER_ERROR_INVALID_DESTINATION;
}

HandlerErrorCode DefaultCecHandler::ValidateSendCommand(std::shared_ptr<Command> command) {
  std::shared_ptr<SendCommandReqData> commandData = std::static_pointer_cast<SendCommandReqData>(command->getData());

  if (!commandData->adapter.empty()) {
    if (ValidateAdapter(commandData->adapter) != HANDLER_ERROR_OK)
      return HANDLER_ERROR_INVALID_ADAPTER;
  }

  if (ValidateAddress(commandData->destAddress) != HANDLER_ERROR_OK)
    return HANDLER_ERROR_INVALID_DESTINATION;

  if (commandData->command.name == "report-power-status") {

   if (commandData->command.args.size() != 1)
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().arg != "pwr-state")
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().value != "on" && commandData->command.args.front().value != "standby" && !commandData->command.args.front().value.empty())
     return HANDLER_ERROR_INVALID_PARAMTERS;

  } else if(commandData->command.name == "report-audio-status") {

   if (commandData->command.args.size() != 1)
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().arg != "aud-mute-status")
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().value != "on" && commandData->command.args.front().value != "off" && !commandData->command.args.front().value.empty())
     return HANDLER_ERROR_INVALID_PARAMTERS;

  } else if(commandData->command.name == "set-volume") {

   if (commandData->command.args.size() != 1)
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().arg != "volume")
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().value != "up" && commandData->command.args.front().value != "down" && !commandData->command.args.front().value.empty())
     return HANDLER_ERROR_INVALID_PARAMTERS;

  } else if(commandData->command.name == "osd-display") {

   if (commandData->command.args.size() != 1)
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().arg != "osd")
     return HANDLER_ERROR_INVALID_PARAMTERS;

  } else if(commandData->command.name == "active") {

   if (commandData->command.args.size() != 1)
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().arg != "set-active")
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (!commandData->command.args.front().value.empty())
     return HANDLER_ERROR_INVALID_PARAMTERS;

  } else if(commandData->command.name == "one-touch-play") {

   if (commandData->command.args.size() != 1)
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().arg != "active-source")
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (!commandData->command.args.front().value.empty())
     return HANDLER_ERROR_INVALID_PARAMTERS;

  } else if(commandData->command.name == "system-information") {

    for (auto it = commandData->command.args.begin();  it!=commandData->command.args.end(); ++it) {

      if ((*it).arg != "vendor-id" && (*it).arg != "version"  && (*it).arg != "name" && (*it).arg != "language" && (*it).arg != "is-active")
       return HANDLER_ERROR_INVALID_PARAMTERS;

      if (!((*it).value.empty()))
       return HANDLER_ERROR_INVALID_PARAMTERS;
    }

  } else if(commandData->command.name == "vendor-commands") {

   if (commandData->command.args.size() != 1)
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().arg != "payload")
     return HANDLER_ERROR_INVALID_PARAMTERS;

   if (commandData->command.args.front().value.empty())
     return HANDLER_ERROR_INVALID_PARAMTERS;

  } else {

    return HANDLER_ERROR_INVALID_COMMAND;

  }

  return HANDLER_ERROR_OK;
}

HandlerErrorCode DefaultCecHandler::ValidateListAdapters(std::shared_ptr<Command> command) {
  return HANDLER_ERROR_OK;
}

HandlerErrorCode DefaultCecHandler::ValidateScan(std::shared_ptr<Command> command) {
  std::shared_ptr<ScanReqData> scanData = std::static_pointer_cast<ScanReqData>(command->getData());

  if (!scanData->adapter.empty()) {
    if (ValidateAdapter(scanData->adapter) != HANDLER_ERROR_OK)
      return HANDLER_ERROR_INVALID_ADAPTER;
  }
  return HANDLER_ERROR_OK;
}

HandlerErrorCode DefaultCecHandler::ValidateGetConfig(std::shared_ptr<Command> command) {
  std::shared_ptr<GetConfigReqData> configData = std::static_pointer_cast<GetConfigReqData>(command->getData());

  if (!configData->adapter.empty()) {
    if (ValidateAdapter(configData->adapter) != HANDLER_ERROR_OK)
      return HANDLER_ERROR_INVALID_ADAPTER;
  }
  if (configData->key == "vendorId")
    return HANDLER_ERROR_OK;
  else if (configData->key == "version")
    return HANDLER_ERROR_OK;
  else if (configData->key == "osd")
    return HANDLER_ERROR_OK;
  else if (configData->key == "language")
    return HANDLER_ERROR_OK;
  else if (configData->key == "powerState")
    return HANDLER_ERROR_OK;
  else if (configData->key == "physicalAddress")
    return HANDLER_ERROR_OK;
  else if (configData->key == "logicalAddress")
    return HANDLER_ERROR_OK;
  else
    return HANDLER_ERROR_INVALID_PARAMTERS;

  return HANDLER_ERROR_OK;
}

HandlerErrorCode DefaultCecHandler::ValidateSetConfig(std::shared_ptr<Command> command) {
  std::shared_ptr<SetConfigReqData> configData = std::static_pointer_cast<SetConfigReqData>(command->getData());

  if (!configData->adapter.empty()) {
    if (ValidateAdapter(configData->adapter) != HANDLER_ERROR_OK)
      return HANDLER_ERROR_INVALID_ADAPTER;
  }
  if (configData->key == "vendorId")
    return HANDLER_ERROR_OK;
  else if (configData->key == "osd")
    return HANDLER_ERROR_OK;
  else if (configData->key == "deviceType")
    return HANDLER_ERROR_OK;
  else
    return HANDLER_ERROR_INVALID_PARAMTERS;

  return HANDLER_ERROR_OK;
}

