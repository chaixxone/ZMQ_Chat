#pragma once

#include <optional>
#include <memory>

#include "client_interface.hpp"
#include "message_view.hpp"
#include "message_observer_interface.hpp"

class IClientMessageSubject : public IClient
{
public:
	virtual ~IClientMessageSubject() = default;
	virtual void AttachMessageObserver(std::shared_ptr<IMessageObserver> messageObserver) = 0;
	virtual std::optional<MessageView> TryGetMessage() = 0;
};