#include "server.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Server::Server(zmq::context_t& context, std::string binding, std::unique_ptr<DatabaseConnection> dbConn) :
    _context(context), 
    _socket(_context, zmq::socket_type::router), 
    _databaseConnection(std::move(dbConn)),
    _running(true)
{
    char* zmqPublicKey = std::getenv("SERVER_PUBLIC_KEY");
    char* zmqSecretKey = std::getenv("SERVER_SECRET_KEY");

    _socket.set(zmq::sockopt::curve_server, true);
    _socket.set(zmq::sockopt::curve_secretkey, zmqSecretKey);
    _socket.set(zmq::sockopt::curve_publickey, zmqPublicKey);
    _socket.bind(binding);
}

void Server::Run()
{
    while (_running)
    {
        try
        {
            zmq::message_t identity, action, data, chatId;
            auto identityResult = _socket.recv(identity, zmq::recv_flags::none);
            auto actionResult = _socket.recv(action, zmq::recv_flags::none);
            auto dataResult = _socket.recv(data, zmq::recv_flags::none);
            auto chatIdResult = _socket.recv(chatId, zmq::recv_flags::none);

            std::string clientId = identity.to_string();
            std::string actionStr = action.to_string();
            std::string dataStr = data.to_string();
            std::string chatIdStr = chatId.to_string();
            int chatIdNumber = std::stoi(chatIdStr);
            std::cout << clientId << " " << actionStr << " " << dataStr << chatIdNumber << '\n';

            Utils::Action actionEnum = Utils::stringToAction(actionStr);

            switch (actionEnum)
            {
            case Utils::Action::Connect:
                HandleConnection(identity, dataStr);
                break;
            case Utils::Action::ChangeName:
                HandleConnection(identity, dataStr);
                break;
            case Utils::Action::Register:
                HandleRegister(clientId, dataStr);
                break;
            case Utils::Action::SendMessage:
                HandleSendMessage(clientId, dataStr, chatIdNumber);
                break;
            case Utils::Action::CreateChat:
                PrepareNewChatSession(clientId, dataStr);
                break;
            case Utils::Action::AcceptCreateChat:
                HandleResponseForInvite(identity, clientId, dataStr, true);
                break;
            case Utils::Action::DeclineCreateChat:
                HandleResponseForInvite(identity, clientId, dataStr, false);
                break;
            case Utils::Action::AllChats:
                HandleAllChatsInfoRequest(clientId);
                break;
            case Utils::Action::ClientChats:
                HandleClientChatsInfoRequest(clientId);
                break;
            case Utils::Action::Invites:
                HandleClientPendingInvites(clientId);
                break;
            case Utils::Action::ClientsByName:
                HandleGetClientsByName(clientId, dataStr);
                break;
            default:
                std::cout << "Uknown action appeared\n";
                break;
            }
        }
        catch (const zmq::error_t& error)
        {
            if (error.num() == ETERM)
            {
                _running = false;
            }
        }
    }
}

void Server::HandleConnection(zmq::message_t& clientId, const std::string& desiredIdentity)
{
    if (_clients.find(desiredIdentity) != _clients.end())
    {
        MessageDispatch(Utils::Action::Unknown, desiredIdentity, clientId.to_string());
        return;
    }

    MessageDispatch(Utils::Action::NewClientName, desiredIdentity, clientId.to_string());

    _clients.insert(desiredIdentity);
    std::cout << "[Server] Client " << desiredIdentity << " connected.\n";
}

void Server::HandleRegister(const std::string& clientId, const std::string& data)
{
    std::string successfulRegisterMessage = "Successfully registrated";
    std::string failedRegisterMessage = "Couldn't register account, login is already used";

    json userData;
    std::string desiredLogin;
    std::string password;
    std::string passwordRepeat;

    try
    {
        userData = json::parse(data);
        desiredLogin = userData["login"].get<std::string>();
        password = userData["password"].get<std::string>();
        passwordRepeat = userData["password_repeat"].get<std::string>();
    }
    catch (const json::exception& e)
    {
        std::cerr << "[Server] JSON parsing failed at registration: " << e.what() << '\n';
        return;
    }

    json registrationStatus;

    if (password != passwordRepeat)
    {
        registrationStatus = { { "message", "passwords are mismatching" }, {"is_registered", false } };
        MessageDispatch(Utils::Action::Register, registrationStatus.dump(), clientId);
        return;
    }
    else if (!_databaseConnection->RegisterUser(desiredLogin, password))
    {
        registrationStatus = { { "message", failedRegisterMessage }, {"is_registered", false } };
        MessageDispatch(Utils::Action::Register, registrationStatus.dump(), clientId);
        return;
    }

    registrationStatus = { { "message", successfulRegisterMessage }, {"is_registered", true } };

    MessageDispatch(Utils::Action::Register, registrationStatus.dump(), clientId);
}

void Server::HandleSendMessage(const std::string& clientId, const std::string& dataStr, int chatId)
{
    size_t delimiter = dataStr.find_first_of(":");

    static size_t messageId = 0;

    if (chatId == -1)
    {
        std::cerr << "[Server] Refusing to take message from " << clientId << ": no correct chat id in dataFrame\n";
        return;
    }

    MessageDispatch(Utils::Action::IncomingMessage, dataStr, _activeChats[chatId], std::to_string(messageId++), clientId, chatId);
}

void Server::PrepareNewChatSession(const std::string& clientId, const std::string& dataStr)
{
    static int chatIdCounter = 0;
    std::unordered_set<std::string> clients = ParseClients(dataStr, clientId);

    std::cout << "[Server] Client " << clientId << " asked to create a chat with " << dataStr << '\n';

    AskClients(chatIdCounter, clientId, clients);
    _activeChats[chatIdCounter].insert(clientId);
    MessageDispatch(Utils::Action::NewChat, std::to_string(chatIdCounter), clientId);
    chatIdCounter++;
}

void Server::HandleResponseForInvite(zmq::message_t& identity, const std::string& clientId, const std::string& dataStr, bool isAccepted)
{
    int chatId = std::stoi(dataStr);

    if (_pendingChatInvites.find(chatId) != _pendingChatInvites.end())
    {
        _pendingChatInvites[chatId].erase(clientId);

        if (!isAccepted)
        {
            std::cout << "[Server] Client " << clientId << " declined chat invitation.\n";
            return;
        }

        _activeChats[chatId].insert(clientId);
        std::cout << "[Server] Client " << clientId << " accepted chat invitation.\n";

        MessageDispatch(Utils::Action::NewChat, std::to_string(chatId), clientId);
    }
}

void Server::HandleAllChatsInfoRequest(const std::string& clientId)
{
    std::vector<std::string> allChatsIdVector;

    for (const auto& chat : _activeChats)
    {
        allChatsIdVector.push_back(std::to_string(chat.first));
    }

    json allChatsJson;
    allChatsJson = allChatsIdVector;

    MessageDispatch(Utils::Action::AllChats, allChatsJson.dump(), clientId);
}

void Server::HandleClientChatsInfoRequest(const std::string& clientId)
{
    std::vector<std::string> clientChatsIdVector;

    // BAD, yet temporary solution!
    for (const auto& chat : _activeChats)
    {
        if (chat.second.find(clientId) != chat.second.end())
        {
            clientChatsIdVector.push_back(std::to_string(chat.first));
        }
    }

    json clientChatsJson;
    clientChatsJson = clientChatsIdVector;
    MessageDispatch(Utils::Action::ClientChats, clientChatsJson.dump(), clientId);
}

std::unordered_set<std::string> Server::ParseClients(const std::string& clients, const std::string& creator)
{
    std::unordered_set<std::string> clientSet;
    std::stringstream ss(clients);
    std::string client;
    while (std::getline(ss, client, ' '))
    {
        if (client != creator)
        {
            clientSet.insert(client);
        }
    }
    return clientSet;
}

void Server::AskClients(int PendingInvitesChatId, const std::string& creator, const std::unordered_set<std::string>& clients)
{
    MessageDispatch(Utils::Action::CreateChat, creator, clients, "", creator, PendingInvitesChatId);

    for (const auto& client : clients)
    {
        if (_clients.find(client) != _clients.end())
        {
            _pendingChatInvites[PendingInvitesChatId].insert(client);
        }
        else
        {
            std::cerr << "[Server] Client " << client << " doesn't exist\n";
        }
    }
}

void Server::MessageDispatch(Utils::Action action, const std::string& message, const std::string& clientId)
{
    const std::string defaultChatId = "-1";
    zmq::message_t clientIdFrame(clientId);
    zmq::message_t actionFrame(Utils::actionToString(action));
    zmq::message_t data(message);
    // create empty frames
    zmq::message_t messageId(0);
    zmq::message_t author(0);

    zmq::message_t chatId(defaultChatId);

    _socket.send(clientIdFrame, zmq::send_flags::sndmore);
    _socket.send(actionFrame, zmq::send_flags::sndmore);
    _socket.send(data, zmq::send_flags::sndmore);
    _socket.send(messageId, zmq::send_flags::sndmore);
    _socket.send(author, zmq::send_flags::sndmore);
    _socket.send(chatId, zmq::send_flags::none);
}

void Server::MessageDispatch(
    Utils::Action action,
    const std::string& message, 
    const std::unordered_set<std::string>& clients,
    const std::string& messageIdStr,
    const std::string& authorStr,
    int chatIdInt
)
{
    for (const auto& client : clients)
    {
        zmq::message_t clientId(client);
        zmq::message_t actionFrame(Utils::actionToString(action));
        zmq::message_t data(message);
        zmq::message_t messageId(messageIdStr);
        zmq::message_t author(authorStr);
        zmq::message_t chatId(std::to_string(chatIdInt));

        _socket.send(clientId, zmq::send_flags::sndmore);
        _socket.send(actionFrame, zmq::send_flags::sndmore);
        _socket.send(data, zmq::send_flags::sndmore);
        _socket.send(messageId, zmq::send_flags::sndmore);
        _socket.send(author, zmq::send_flags::sndmore);
        _socket.send(chatId, zmq::send_flags::none);
    }
}

void Server::HandleGetClientsByName(const std::string& clientId, const std::string& name)
{
    std::vector<std::string> suggestedClientNames;

    for (const auto& identifier : _clients)
    {
        if (identifier.contains(name))
        {
            suggestedClientNames.push_back(identifier);
        }
    }

    json clientNamesData = suggestedClientNames;
    MessageDispatch(Utils::Action::ClientsByName, clientNamesData.dump(), clientId);
}

void Server::HandleClientPendingInvites(const std::string& clientId)
{
    std::vector<int> clientChatInvites;

    for (const auto& [chatId, invitedClients] : _pendingChatInvites)
    {
        if (invitedClients.contains(clientId))
        {
            clientChatInvites.push_back(chatId);
        }
    }

    json clientInvitesData;
    clientInvitesData = clientChatInvites;

    MessageDispatch(Utils::Action::Invites, clientInvitesData.dump(), clientId);
}