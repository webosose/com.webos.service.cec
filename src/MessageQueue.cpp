#include "MessageQueue.h"

MessageQueue::MessageQueue()
	: mQuit(false)
{
	mThread = std::thread(std::bind(&MessageQueue::dispatchMessage, this));
	mThread.detach();
}

MessageQueue::~MessageQueue()
{
	mQuit = true;
	if (mThread.joinable())
	{
		mThread.join();
	}
}


void MessageQueue::sampleCallback(std::string response)
{
	//std::cout<<"Recieved Response : "<<response<<std::endl;
}

bool MessageQueue::handleMessage(std::shared_ptr<nyx_cec_command> request)
{
	AppLogInfo() <<__func__ << "\n";
	//std::cout << "Entering "<<__func__ << std::endl;
	setCallback(std::bind(&MessageQueue::sampleCallback, this, std::placeholders::_1));
	call.nyx_cec_send_command(request);
	return true;
}

void MessageQueue::addMessage(std::shared_ptr<nyx_cec_command> request)
{
	AppLogInfo() <<__func__ << "\n";
	//std::cout << "Entering "<<__func__ << std::endl;
	mQueue.push_back(request);
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
			lock.unlock();
			if (handleMessage(std::move(mQueue.front())))
				mQueue.erase(mQueue.begin());
			lock.lock();
		}
	} while (!mQuit);
}

