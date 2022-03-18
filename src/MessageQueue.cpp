#include "MessageQueue.h"

//Need to check
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
    AppLogInfo() <<__func__ << "Received :\n";
    std::vector<std::string> resp;
    for(int i=0;i<response->size;i++)
    {
        AppLogInfo() <<response->responses[i]<<"\n";
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
    if(request->type == CommandType::SCAN)
        strcpy(command.name,"scan");
    else if(request->type == CommandType::LIST_ADAPTERS)
        strcpy(command.name,"listAdapters");
    else if(request->params.find("name") != request->params.end())
        strcpy(command.name,request->params["name"].c_str());
    //strcpy(command.name,getMessageString(request->type).c_str());
    int i =0;
    for(auto arg : request->params)
    {
        strcpy(command.params[i].name,arg.first.c_str());
        strcpy(command.params[i].value,arg.second.c_str());
        i++;
    }
    error = nyx_cec_send_command(mDevice, &command);
    if(error != NYX_ERROR_NONE)
        AppLogError() <<__func__<<": Failed with :"<<error<<"\n";
}

void MessageQueue::getConfig(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    mType = request->type;

    nyx_error_t error;
    char *configName = new char;
    char *value = new char; //need to check
    for (auto it : request->params)
    {
        if(it.second.empty())
            strcpy(configName,it.first.c_str());
    }
    error = nyx_cec_get_config(mDevice, configName, &value);
    if(error != NYX_ERROR_NONE)
        AppLogError() <<__func__<<": Failed with :"<<error<<"\n";
    else
    {
        AppLogInfo() <<__func__<<": Value :"<<value<<"\n";
        std::vector<std::string> resp;
        resp.push_back(value);
        mCb(mType,resp);
    }
    delete configName;
    delete value;
}

void MessageQueue::setConfig(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    mType = request->type;

    nyx_error_t error;
    char *type = new char;
    char *value = new char;
     for (auto it : request->params)
    {
        if(it.first != "adapter")
        {
            strcpy(type,it.first.c_str());
            strcpy(value,it.second.c_str());
        }
    }
    error = nyx_cec_set_config(mDevice, type, value);
    if(error != NYX_ERROR_NONE)
        AppLogError() <<__func__<<": Failed with :"<<error<<"\n";
    else
        AppLogInfo() <<__func__<<": Success"<<"\n";
    delete type;
    delete value;
}

bool MessageQueue::handleMessage(std::shared_ptr<MessageData> request)
{
    AppLogInfo() <<__func__ << "\n";
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

