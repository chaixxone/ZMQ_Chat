#pragma once
#include <string>

namespace Utils
{
    enum class Action
    {
        Connect,
        Register,
        Authorize,
        Logout,
        AlreadyAuthorized,
        NotAuthorized,
        ChangeName,
        NewClientName,
        SendMessage,
        IncomingMessage,
        CreateChat,
        AcceptCreateChat,
        DeclineCreateChat,
        NewChat,
        AllChats,
        ClientChats,
        Notifications,
        ClientsByName,
        Unknown
    };

    Action stringToAction(const std::string& actionStr);

    std::string actionToString(Action action);
}
