#pragma once

class IServer
{
public:
	virtual ~IServer() = default;
	virtual void Run() = 0;
};