#pragma once
#include <string>

namespace Utils
{
    enum class Action
    {
        Connect,
        ChangeName,
        SendMessage,
        CreateChat,
        AcceptCreateChat,
        Unknown
    };

    extern const short CREATE_CHAT_PREFIX_LENGTH;

    Action stringToAction(const std::string& actionStr);

    std::string actionToString(Action action);
}
