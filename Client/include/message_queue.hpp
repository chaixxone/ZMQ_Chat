#pragma once

#include <string>
#include <utility>
#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class MessageQueue
{
public:
	MessageQueue() {}
	~MessageQueue() {}

	T Pop()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		
		while (_enqueuedMessages.empty())
		{
			_cv.wait(lock);
		}

		_enqueuedMessages.pop_front();
	}

	bool IsEmpty() const
	{
		std::unique_lock<std::mutex> lock(_mutex);
		bool isEmpty = _enqueuedMessages.empty();
		lock.unlock();
		return isEmpty;
	}

	void Enqueue(T&& item)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_enqueuedMessages.push_back(std::move(item));
		lock.unlock();
		_cv.notify_one();
	}

private:
	std::mutex _mutex;
	std::condition_variable _cv;
	std::deque<T> _enqueuedMessages;
};