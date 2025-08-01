#include <qdebug.h>

#include <qt_message_observer.hpp>

QtMessageObserver::QtMessageObserver()
{
	
}

void QtMessageObserver::Subscribe(std::shared_ptr<IClientMessageSubject> client)
{
	_client = client;
	client->AttachMessageObserver(shared_from_this());
}

QtMessageObserver::~QtMessageObserver()
{

}

void QtMessageObserver::Update()
{
	if (!_client.expired())
	{
		std::shared_ptr<IClientMessageSubject> client = _client.lock();
		std::optional<MessageView> message = client->TryGetMessage();
		qDebug() << message->ChatID << '\t' << message->ID << '\t' << message->Author << '\t' << message->Content;

		MessageView& messageData = message.value();

		switch (messageData.Action)
		{
		case Utils::Action::Register:
			emit Register(messageData);
			break;
		case Utils::Action::Authorize:
			emit Authorize(messageData);
			break;
		case Utils::Action::AlreadyAuthorized:
			emit AlreadyAuthorized(messageData);
			break;
		case Utils::Action::NotAuthorized:
			emit NotAuthorized(messageData);
			break;
		case Utils::Action::IncomingMessage:
			emit IncomingMessage(messageData);
			break;
		case Utils::Action::CreateChat:
			emit CreateChat(messageData);
			break;
		case Utils::Action::NewChat:
			emit NewClientChat(messageData);
			break;
		case Utils::Action::ClientChats:
			emit ClientChats(messageData);
			break;
		case Utils::Action::NewClientName:
			emit NewClientName(QString::fromStdString(messageData.Content));
			break;
		case Utils::Action::ClientsByName:
			emit ClientsByName(messageData.Content);
			break;
		case Utils::Action::Notifications:
			emit Notifications(messageData);
			break;
		default:
			break;
		}
	}
}