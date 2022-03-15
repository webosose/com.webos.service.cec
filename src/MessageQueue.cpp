#include "MessageQueue.h"

MessageQueue::MessageQueue()
    : mQuit(false)
{
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
            AppLogError() <<"Failed to get `Open nyx device: "<< error;
        }
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
    //std::vector<std::string> resp;
    for(int i=0;i<response->size;i++)
        //resp.push_back(response->responses[i]);
        AppLogInfo() <<response->responses[i]<<"\n";
}

void MessageQueue::listAdapters(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
}

void MessageQueue::sendCommand(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    nyx_error_t error;
    nyx_cec_command_t command;
    error = nyx_cec_send_command(mDevice, &command);
    if(error != NYX_ERROR_NONE)
        AppLogError() <<__func__<<": Failed \n";
}

void MessageQueue::getConfig(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    nyx_error_t error;
    char *configName  = nullptr;
    char *value = nullptr;
    error = nyx_cec_get_config(mDevice, configName, &value);
    if(error != NYX_ERROR_NONE)
        AppLogError() <<__func__<<": Failed \n";
    else 
        AppLogInfo() <<__func__<<": Value :"<<*value<<"\n";
}

void MessageQueue::setConfig(std::shared_ptr<MessageData> request)
{
    AppLogDebug() <<__func__<<"\n";
    nyx_error_t error;
    char type[10];
    char value[10];
    error = nyx_cec_set_config(mDevice, type, value);
    if(error != NYX_ERROR_NONE)
        AppLogError() <<__func__<<": Failed \n";
    else 
        AppLogInfo() <<__func__<<": Success :"<<error<<"\n";
}


bool MessageQueue::handleMessage(std::shared_ptr<MessageData> request)
{
    AppLogInfo() <<__func__ << "\n";
    switch(request->type)
    {
        case CommandType::LIST_ADAPTERS:
        {
            AppLogDebug() <<__func__<<":LIST_ADAPTERS CommandType\n";
        }
        break;
        case CommandType::SCAN:
        {
            AppLogDebug() <<__func__<<":SCAN CommandType\n";
            sendCommand(request);
        }
        break;
        case CommandType::SEND_COMMAND:
        {
            AppLogDebug() <<__func__<<":SEND_COMMAND CommandType\n";
            sendCommand(request);
        }
        break;
        case CommandType::GET_CONFIG:
        {
            AppLogDebug() <<__func__<<":GET_CONFIG CommandType\n";
            getConfig(request);
        }
        break;
        case CommandType::SET_CONFIG:
        {
            AppLogDebug() <<__func__<<":SET_CONFIG CommandType\n";
            setConfig(request);
        }
        break;
        default:
            AppLogDebug() <<__func__<<": UNKNOWN CommandType\n";
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

