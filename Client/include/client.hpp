#pragma once
#include "client_message_subject_interface.hpp"
#include "message_observer_interface.hpp"
#include "message_queue.hpp"
#include <thread>
#include <utils/client_actions.hpp>

class Client : public IClientMessageSubject
{
public:
    Client(std::string endpoint, std::string identity, std::shared_ptr<MessageQueue> messageQueue);
    ~Client();
    void RequestToCreateChat(std::string& clients, int chatId) override;
    void SendMessageToChat(std::string& messageStr, int chatIdInt) override;
    bool HasRequestToChat() const;
    void Reply(const std::string& reply);
    void RequestChangeIdentity(std::string& desiredIdentity);
    std::optional<MessageView> TryGetMessage() override;
    int GetChatId() const noexcept;
    void Attach(std::shared_ptr<IMessageObserver> messageObserver) override;

private:
    void SendRequest(std::string& messageStr, Utils::Action action, int chatIdInt);
    void ReceiveMessage();
    static std::string GenerateTemporaryId();
    void ChangeIdentity(const std::string& identity);

    zmq::context_t _context;
    zmq::socket_t _socket;
    std::string _endpoint;
    std::string _identity;
    std::thread _receiver;
    std::shared_ptr<MessageQueue> _messageQueue;
    std::shared_ptr<IMessageObserver> _messageObserver;
    int _pendingChatId;
    int _chatId;
    bool _hasRequestToChat;
    std::atomic_bool _alive = true;
};
