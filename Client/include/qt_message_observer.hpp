#pragma once

#include <memory>

#include <client_interface.hpp>
#include <message_observer_interface.hpp>

class QtMessageObserver : public IMessageObserver
{
public:
	QtMessageObserver(std::shared_ptr<IClient> client);
	~QtMessageObserver();

	void Update() override;

private:
	std::weak_ptr<IClient> _client;
};