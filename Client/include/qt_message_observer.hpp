#pragma once

#include <memory>

#include <client.hpp>
#include <message_observer_interface.hpp>

class QtMessageObserver : public IMessageObserver
{
public:
	QtMessageObserver(std::shared_ptr<Client> client);
	~QtMessageObserver();

	void Update() override;

private:
	std::weak_ptr<Client> _client;
};