#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <glib.h>
#include <functional>
#include <unistd.h>
#include "Logger.h"


class MessageQueue
{
public:
	MessageQueue();
	~MessageQueue();

	void dispatchMessage();
	void sampleCallback(std::string);
	void addMessage(std::shared_ptr<nyx_cec_command>);
	bool handleMessage(std::shared_ptr<nyx_cec_command>);
	void sampleCallback(std::string);



private:
	std::vector<std::shared_ptr<nyx_cec_command>> mQueue;
	std::thread mThread;
	std::mutex mMutex;
	std::condition_variable mCondVar;
	volatile bool mQuit;
	CecNyxInterface call;
};

