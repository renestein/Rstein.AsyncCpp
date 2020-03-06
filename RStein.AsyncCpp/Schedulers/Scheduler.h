#pragma once
#include <memory>
#include <functional>

class Scheduler : public std::enable_shared_from_this<Scheduler>
{

public:
	Scheduler();
	virtual ~Scheduler();
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void EnqueueItem(std::function<void()> &&originalFunction) = 0;
	virtual bool IsMethodInvocationSerialized() = 0;
};

