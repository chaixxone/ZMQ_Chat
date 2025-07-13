#pragma once

#include <memory>

#include <QObject>

#include <client.hpp>
#include <client_message_subject_interface.hpp>
#include <message_observer_interface.hpp>

class QtMessageObserver : public QObject, public IMessageObserver, public std::enable_shared_from_this<QtMessageObserver>
{
	Q_OBJECT

public:
	QtMessageObserver();
	~QtMessageObserver();

	void Update() override;

	void Subscribe(std::shared_ptr<IClientMessageSubject> client);

signals:
	void IncomingMessage(const MessageView& message);

	void CreateChat(const MessageView& message);

	void NewClientName(const QString& newClientName);

	void NewClientChat(const MessageView& message);

	void ClientChats(const MessageView& message);

	void ClientsByName(const QString& clientsStr);

private:
	std::weak_ptr<IClientMessageSubject> _client;

	void ProcessMessageActions(const MessageView& message);
};