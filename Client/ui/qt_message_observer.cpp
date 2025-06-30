#include <qt_message_observer.hpp>

QtMessageObserver::QtMessageObserver(std::shared_ptr<Client> client) : _client(client)
{
	client->Attach(shared_from_this());
}

QtMessageObserver::~QtMessageObserver()
{

}

void QtMessageObserver::Update()
{
	if (!_client.expired())
	{
		std::shared_ptr<Client> client = _client.lock();
		std::optional<MessageView> message = client->TryGetMessage();
	}
}