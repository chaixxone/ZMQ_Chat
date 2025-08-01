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
    _nonSessionActions.insert(Utils::Action::Connect);
    _nonSessionActions.insert(Utils::Action::Authorize);
    _nonSessionActions.insert(Utils::Action::Register);

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
            zmq::message_t identity, deviceID, sessionID, action, data, chatId;
            auto identityResult = _socket.recv(identity, zmq::recv_flags::none);
            auto deviceIDResult = _socket.recv(deviceID, zmq::recv_flags::none);
            auto sessionIDResult = _socket.recv(sessionID, zmq::recv_flags::none);
            auto actionResult = _socket.recv(action, zmq::recv_flags::none);
            auto dataResult = _socket.recv(data, zmq::recv_flags::none);
            auto chatIdResult = _socket.recv(chatId, zmq::recv_flags::none);

            std::string clientId = identity.to_string();
            std::string deviceIDStr = deviceID.to_string();
            std::string sessionIDStr = sessionID.to_string();
            std::string actionStr = action.to_string();
            std::string dataStr = data.to_string();
            std::string chatIdStr = chatId.to_string();
            int chatIdNumber = std::stoi(chatIdStr);
            std::cout << clientId << " " << actionStr << " " << dataStr << chatIdNumber << '\n';

            Utils::Action actionEnum = Utils::stringToAction(actionStr);

            if (_nonSessionActions.contains(actionEnum))
            {
                switch (actionEnum)
                {
                case Utils::Action::Connect:
                    HandleConnection(clientId, dataStr, deviceIDStr);
                    break;
                case Utils::Action::Register:
                    HandleRegister(clientId, dataStr);
                    break;
                case Utils::Action::Authorize:
                    HandleAuthorize(clientId, dataStr, deviceIDStr);
                    break;
                default:
                    break;
                }
            }
            else
            {
                if (!_databaseConnection->DoesSessionExist(clientId, deviceIDStr, sessionIDStr))
                {
                    MessageDispatch(Utils::Action::NotAuthorized, "Error: you're not allowed to do anything without logging in", clientId);
                    continue;
                }

                switch (actionEnum)
                {
                case Utils::Action::Logout:
                    HandleLogout(clientId, sessionIDStr, deviceIDStr);
                    break;
                case Utils::Action::ChangeName:
                    HandleConnection(clientId, dataStr, deviceIDStr);
                    break;
                case Utils::Action::SendMessage:
                    HandleSendMessage(clientId, dataStr, chatIdNumber);
                    break;
                case Utils::Action::CreateChat:
                    PrepareNewChatSession(clientId, dataStr);
                    break;
                case Utils::Action::AcceptCreateChat:
                    HandleResponseForInvite(clientId, dataStr, true);
                    break;
                case Utils::Action::DeclineCreateChat:
                    HandleResponseForInvite(clientId, dataStr, false);
                    break;
                case Utils::Action::ClientChats:
                    HandleClientChatsInfoRequest(clientId);
                    break;
                case Utils::Action::Notifications:
                    HandleClientNotifications(clientId);
                    break;
                case Utils::Action::ClientsByName:
                    HandleGetClientsByName(clientId, dataStr);
                    break;
                default:
                    std::cout << "Uknown action appeared\n";
                    break;
                }
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

void Server::HandleConnection(const std::string& generatedClientId, const std::string& clientId, const std::string& deviceID)
{
    if (_databaseConnection->UserDeviceSession(clientId, deviceID))
    {
        MessageDispatch(Utils::Action::AlreadyAuthorized, " ", generatedClientId);
        MessageDispatch(Utils::Action::NewClientName, clientId, generatedClientId);
        std::cout << "[Server] Client " << clientId << " connected.\n";
    }   
    else
    {
        std::string notAuthorizedMessage = "Please authorize first";
        MessageDispatch(Utils::Action::NotAuthorized, notAuthorizedMessage, generatedClientId);
    }
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

void Server::HandleAuthorize(const std::string& clientId, const std::string& dataStr, const std::string deviceID)
{
    std::string successLoginMessage = "Successfully authorized";
    std::string failedLoginMessage = "Authorization failed, incorrect data";

    json userData;
    std::string login;
    std::string password;

    try
    {
        userData = json::parse(dataStr);
        login = userData["login"].get<std::string>();
        password = userData["password"].get<std::string>();
    }
    catch (const json::exception& e)
    {
        std::cerr << "[Server] JSON parsing failed at authorization: " << e.what() << '\n';
        return;
    }

    json authorizeStatus;

    // TODO: update timer if authorize requested before it ends
    std::string sessionID = _databaseConnection->AuthorizeUser(login, password, deviceID);

    if (sessionID.empty())
    {
        authorizeStatus = { 
            { "message", failedLoginMessage }, 
            { "is_authorized", false } 
        };
        MessageDispatch(Utils::Action::Authorize, authorizeStatus.dump(), clientId);
        return;
    }

    authorizeStatus = { 
        { "message", successLoginMessage }, 
        { "is_authorized", true }, 
        { "session_id", sessionID } 
    };

    MessageDispatch(Utils::Action::Authorize, authorizeStatus.dump(), clientId);
    MessageDispatch(Utils::Action::NewClientName, login, clientId);
}

void Server::HandleLogout(const std::string& clientId, const std::string& sessionId, const std::string& deviceID)
{
    _databaseConnection->DeleteSession(clientId, deviceID, sessionId);
    MessageDispatch(Utils::Action::NotAuthorized, "logged out", clientId);
}

void Server::HandleSendMessage(const std::string& clientId, const std::string& dataStr, int chatId)
{
    if (chatId == -1)
    {
        std::cerr << "[Server] Refusing to take message from " << clientId << ": no correct chat id in dataFrame\n";
        return;
    }

    size_t messageID = _databaseConnection->StoreMessage(chatId, dataStr);
    // Send message to active clients
    std::unordered_set<std::string> chatClients = _databaseConnection->GetChatClients(chatId);
    MessageDispatch(Utils::Action::IncomingMessage, dataStr, chatClients, std::to_string(messageID), clientId, chatId);
}

void Server::PrepareNewChatSession(const std::string& clientId, const std::string& dataStr)
{
    std::unordered_set<std::string> clients = ParseClients(dataStr, clientId);

    std::cout << "[Server] Client " << clientId << " asked to create a chat with " << dataStr << '\n';
    
    int chatId = _databaseConnection->CreateChat();
    AskClients(chatId, clientId, clients);
    _databaseConnection->AddClientToChat(clientId, chatId);

    MessageDispatch(Utils::Action::NewChat, std::to_string(chatId), clientId);
}

void Server::HandleResponseForInvite(const std::string& clientId, const std::string& dataStr, bool isAccepted)
{
    int notificationID = -1;
    int chatId = -1;

    try
    {
        json clientData = json::parse(dataStr);
        notificationID = clientData["notification_id"].get<int>();
        chatId = clientData["invite_chat_id"].get<int>();
    }
    catch (const json::exception& e)
    {
        std::cerr << "[Server] JSON parsing failed at handling response for invite: " << e.what() << '\n';
        return;
    }

    if (isAccepted)
    {
        _databaseConnection->AddClientToChat(clientId, chatId);
        std::cout << "[Server] Client " << clientId << " accepted chat invitation.\n";
        MessageDispatch(Utils::Action::NewChat, std::to_string(chatId), clientId);
    }
    else
    {
        std::cout << "[Server] Client " << clientId << " declined chat invitation.\n";
    }

    _databaseConnection->SetNotificationChecked(notificationID);
}

void Server::HandleClientChatsInfoRequest(const std::string& clientId)
{
    std::vector<int> clientChatsIdVector = _databaseConnection->GetClientChats(clientId);
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

void Server::AskClients(int pendingInvitesChatId, const std::string& creator, const std::unordered_set<std::string>& clients)
{
    for (const auto& client : clients)
    {
        if (_databaseConnection->DoesUserExist(client))
        {
            std::string notificationType = Utils::actionToString(Utils::Action::CreateChat);
            int notificationID = _databaseConnection->AddNotification(creator, client, notificationType, creator, pendingInvitesChatId);
            json inviteData = { { "notification_id", notificationID }, { "author", creator }, { "chat_id", pendingInvitesChatId } };
            MessageDispatch(Utils::Action::CreateChat, inviteData.dump(), client);
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
    std::vector<std::string> suggestedClientNames = _databaseConnection->GetClientsRegexp(clientId, name);
    json clientNamesData = suggestedClientNames;
    MessageDispatch(Utils::Action::ClientsByName, clientNamesData.dump(), clientId);
}

void Server::HandleClientNotifications(const std::string& clientId)
{
    std::vector<json> clientNotifications = _databaseConnection->GetClientNotifications(clientId);

    json clientInvitesData;
    clientInvitesData = clientNotifications;

    MessageDispatch(Utils::Action::Notifications, clientInvitesData.dump(), clientId);
}