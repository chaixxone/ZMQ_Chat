#pragma once

#include <memory>

#include <QObject>

#include <client.hpp>
#include <message_observer_interface.hpp>

class QtMessageObserver : public QObject, public IMessageObserver, public std::enable_shared_from_this<QtMessageObserver>
{
	Q_OBJECT

public:
	QtMessageObserver();
	~QtMessageObserver();

	void Update() override;

	void Subscribe(std::shared_ptr<Client> client);

signals:
	void IncomingMessage(const MessageView& message);

	void CreateChat(const MessageView& message);

	void NewClientName(const MessageView& message);

	void NewClientChat(const MessageView& message);

private:
	std::weak_ptr<Client> _client;

	void ProcessMessageActions(const MessageView& message);
};