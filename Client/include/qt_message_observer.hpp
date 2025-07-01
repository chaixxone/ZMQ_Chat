#pragma once

#include <memory>

#include <QObject>

#include <client.hpp>
#include <message_observer_interface.hpp>

class QtMessageObserver : public IMessageObserver, public std::enable_shared_from_this<QtMessageObserver>, public QObject
{
	Q_OBJECT

public:
	QtMessageObserver();
	~QtMessageObserver();

	void Update() override;

	void Subscribe(std::shared_ptr<Client> client);

private:
	std::weak_ptr<Client> _client;
};