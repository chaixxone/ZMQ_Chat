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
        NewChat,
        Unknown
    };

    extern const short CREATE_CHAT_PREFIX_LENGTH;

    Action stringToAction(const std::string& actionStr);

    std::string actionToString(Action action);
}
