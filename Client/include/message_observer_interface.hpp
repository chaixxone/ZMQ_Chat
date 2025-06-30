#pragma once

class IMessageObserver
{
public:
	virtual ~IMessageObserver() = default;
	virtual void Update() = 0;
};