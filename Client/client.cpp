#include <client.hpp>

#include <iostream>
#include <random>
#include <fstream>

#include <nlohmann/json.hpp>

#include <utils/client_actions.hpp>
#include <utils/helpers.hpp>

using json = nlohmann::json;

Client::Client(std::string endpoint, 
    std::string identity, 
    std::shared_ptr<MessageQueue> message_queue, 
    const std::string& pathToConfigFile
) :
    _context(1), 
    _socket(_context, zmq::socket_type::dealer), 
    _endpoint(endpoint), 
    _identity(GenerateTemporaryId()), 
    _configFilePath(pathToConfigFile),
    _messageQueue(message_queue),
    _chatId(-1),
    _hasRequestToChat(false)
{   
    char clientPublicKey[Utils::CURVE_KEY_LENGTH];
    char clientSecretKey[Utils::CURVE_KEY_LENGTH];
    int keyGenerationResult = zmq_curve_keypair(clientPublicKey, clientSecretKey);

    if (keyGenerationResult != 0)
    {
        throw std::runtime_error("Couldn't generate client keys");
    }

    std::ifstream configFile(_configFilePath);
    
    if (!configFile.is_open())
    {
        throw std::runtime_error("Couldn't open config file");    
    }

    json configFileJson = json::parse(configFile);
    configFile.close();
    std::string serverPublicKey = configFileJson["server_public_key"].get<std::string>();

    if (!configFileJson.contains("device_id"))
    {
        std::string deviceID = Utils::GenerateString();
        configFileJson["device_id"] = deviceID;

        std::ofstream configFileOut(_configFilePath);
        configFileOut << configFileJson.dump(4);
        configFileOut.close();
    }

    std::string desiredIdentity = identity;
    
    if (configFileJson.contains("identity") && !configFileJson["identity"].is_null())
    {
        desiredIdentity = configFileJson["identity"].get<std::string>();
    }

    _deviceID = configFileJson["device_id"].get<std::string>();

    _socket.set(zmq::sockopt::routing_id, _identity);
    _socket.set(zmq::sockopt::linger, 0);
    _socket.set(zmq::sockopt::curve_serverkey, serverPublicKey);
    _socket.set(zmq::sockopt::curve_publickey, clientPublicKey);
    _socket.set(zmq::sockopt::curve_secretkey, clientSecretKey);
    
    _socket.connect(endpoint);

    SendRequest(desiredIdentity, Utils::Action::Connect, -1);

    _receiver = std::thread(&Client::ReceiveMessage, this);
}

void Client::AttachMessageObserver(std::shared_ptr<IMessageObserver> messageObserver)
{
    _messageObserver = messageObserver;
}

void Client::RequestRegister(const std::string identity, const std::string& password, const std::string& passwordRepeat)
{
    json clientData = { { "login", identity }, { "password", password }, { "password_repeat", passwordRepeat } };
    SendRequest(clientData.dump(), Utils::Action::Register, -1);
}

void Client::RequestAuthorize(const std::string& identity, const std::string& password)
{
    json clientData = { { "login", identity }, { "password", password } };
    SendRequest(clientData.dump(), Utils::Action::Authorize, -1);
}

void Client::RequestLogout()
{
    SendRequest("", Utils::Action::Logout, -1);
}

void Client::RequestChangeIdentity(const std::string& desiredIdentity)
{    
    SendRequest(desiredIdentity, Utils::Action::ChangeName, -1);
}

void Client::UpdateClientIDConfig(const std::string& pathToConfig, const std::string& identity)
{
    std::fstream configFile(pathToConfig, std::ios::in);

    if (!configFile.is_open())
    {
        std::cerr << "Couldn't open config file\n";
        return;
    }

    json configFileJson = json::parse(configFile);
    configFile.close();

    configFileJson["identity"] = identity;

    configFile.open(pathToConfig, std::ios::out);
    configFile << configFileJson.dump(4);
    configFile.close();
}

void Client::ChangeIdentity(const std::string& identity)
{
    _identity = identity;
    UpdateClientIDConfig(_configFilePath, identity);
    _socket.disconnect(_endpoint);
    _socket.set(zmq::sockopt::routing_id, _identity);
    _socket.connect(_endpoint);
}

std::string Client::GenerateTemporaryId()
{
    static std::string alphanum =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t size = alphanum.size();
    const size_t length = 36;

    std::string temporaryIdentifier;
    temporaryIdentifier.reserve(length);
    std::random_device rd;
    std::mt19937 mt{ rd() };
    std::uniform_int_distribution<size_t> dist(0, size - 1);

    for (size_t i = 0; i < length; i++) 
    {
        temporaryIdentifier += alphanum[dist(mt)];
    }

    return temporaryIdentifier;
}

void Client::UpdateSessionID(const std::string sessionID, const std::string& pathToConfig)
{
    std::fstream configFile(pathToConfig, std::ios::in);

    if (!configFile.is_open())
    {
        std::cerr << "Couldn't open config file\n";
        return;
    }

    json configFileJson = json::parse(configFile);
    configFile.close();

    configFileJson["session_id"] = sessionID;

    configFile.open(pathToConfig, std::ios::out);
    configFile << configFileJson.dump(4);
    configFile.close();
}

std::string Client::ReadSessionID(const std::string& pathToConfig)
{
    std::ifstream configFile(pathToConfig);

    if (!configFile.is_open())
    {
        return "";
    }

    json configFileJson = json::parse(configFile);

    if (!configFileJson.contains("session_id") || configFileJson["session_id"].is_null())
    {
        return "";
    }

    std::string sessionID = configFileJson["session_id"].get<std::string>();
    return sessionID;
}

Client::~Client()
{
    _alive = false;
    if (_receiver.joinable())
    {
        _receiver.join();
    }
}

bool Client::HasRequestToChat() const
{
    return _hasRequestToChat;
}

int Client::GetChatId() const noexcept
{
    return _chatId;
}

void Client::SendRequest(const std::string& messageStr, Utils::Action action, int chatIdInt)
{
    std::string actionStr = Utils::actionToString(action);
    std::string sessionID = ReadSessionID(_configFilePath);

    zmq::message_t deviceIdFrame(_deviceID);
    zmq::message_t sessionIdFrame(sessionID);
    zmq::message_t actionFrame(actionStr);
    zmq::message_t message(messageStr);
    zmq::message_t chatId(std::to_string(chatIdInt));

    bool result = _socket.send(deviceIdFrame, zmq::send_flags::sndmore)
        && _socket.send(sessionIdFrame, zmq::send_flags::sndmore)
        && _socket.send(actionFrame, zmq::send_flags::sndmore) 
        && _socket.send(message, zmq::send_flags::sndmore)
        && _socket.send(chatId, zmq::send_flags::none);

    if (!result)
    {
        std::cerr << "Failed to send message to chat.\n";
    }
}

void Client::SendMessageToChat(const std::string& messageStr, int chatIdInt)
{
    SendRequest(messageStr, Utils::Action::SendMessage, chatIdInt);
}

void Client::RequestToCreateChat(const std::string& clients)
{
    std::cout << "I am requesting: " << clients << ", to create chat\n";
    SendRequest(clients, Utils::Action::CreateChat, -1);
}

std::optional<MessageView> Client::TryGetMessage()
{
    if (_messageQueue->IsEmpty())
    {
        return std::nullopt;
    }

    return _messageQueue->Pop();
}

void Client::ReceiveMessage()
{
    while (_alive)
    {
        zmq::message_t action;
        zmq::message_t data;
        zmq::message_t messageId;
        zmq::message_t author;
        zmq::message_t chatId;
        bool messageReceivedResult = _socket.recv(action, zmq::recv_flags::dontwait)
            && _socket.recv(data, zmq::recv_flags::dontwait)
            && _socket.recv(messageId, zmq::recv_flags::dontwait)
            && _socket.recv(author, zmq::recv_flags::dontwait)
            && _socket.recv(chatId, zmq::recv_flags::dontwait);

        if (messageReceivedResult)
        {
            std::string actionStr = action.to_string();
            std::string dataStr = data.to_string();
            std::string messageIdStr = messageId.to_string();
            std::string authorStr = author.to_string();

            Utils::Action actionEnum = Utils::stringToAction(actionStr);
            int chatIdInt = std::stoi(chatId.to_string());
            std::optional<size_t> messageId = messageIdStr.empty() ? std::nullopt : std::optional(std::stoull(messageIdStr));
            _messageQueue->Enqueue(MessageView{ authorStr, dataStr, messageId, chatIdInt, actionEnum });

            if (_messageObserver)
            {
                _messageObserver->Update();
            }

            switch (actionEnum)
            {
            case Utils::Action::Authorize:
            {
                json authorizeStatus = json::parse(dataStr);
                bool isAuthorized = authorizeStatus["is_authorized"].get<bool>();

                if (isAuthorized)
                {
                    std::string sessionID = authorizeStatus["session_id"].get<std::string>();
                    UpdateSessionID(sessionID, _configFilePath);
                }

                break;
            }
            case Utils::Action::CreateChat:
                std::cout << "[" << _identity << "]" << " I am invited to chat " << chatIdInt << '\n';
                _hasRequestToChat = true;
                _pendingChatId = chatIdInt;
                std::cout << "[Server] Do you wish to create chat with " << dataStr << "? (y/n)\n";
                break;
            case Utils::Action::NewChat:
                _chatId = std::stoi(dataStr);
                std::cout << "[Server] Now you are in chat with id=" << dataStr << '\n';
                break;
            case Utils::Action::IncomingMessage:
                break;
            case Utils::Action::NewClientName:
                ChangeIdentity(dataStr);
                break;
            default:
                std::cout << "Error: unknown action!\n";
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Client::ReplyChatInvite(int chatId, bool isAccepted)
{
    std::string replyPendingChatIDstr = std::to_string(chatId);

    if (isAccepted)
    {
        SendRequest(replyPendingChatIDstr, Utils::Action::AcceptCreateChat, -1);
    }
    else
    {
        SendRequest(replyPendingChatIDstr, Utils::Action::DeclineCreateChat, -1);
    }

    _hasRequestToChat = false;
}

void Client::GetClientChatIdsStr()
{
    SendRequest(_identity, Utils::Action::ClientChats, -1);
}

void Client::GetClientsByName(const std::string& name)
{
    SendRequest(name, Utils::Action::ClientsByName, -1);
}

void Client::GetInvites()
{
    SendRequest("", Utils::Action::Invites, -1);
}