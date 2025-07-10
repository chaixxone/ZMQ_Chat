#pragma once
#include <string>

namespace Utils
{
    enum class Action
    {
        Connect,
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
        ClientsByName,
        Unknown
    };

    Action stringToAction(const std::string& actionStr);

    std::string actionToString(Action action);
}
