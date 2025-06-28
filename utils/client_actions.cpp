#include "client_actions.hpp"
#include <unordered_map>

namespace Utils
{
    const short CREATE_CHAT_PREFIX_LENGTH = 12;

    Action stringToAction(const std::string& actionStr)
    {
        static const std::unordered_map<std::string, Action> actionMap = {
            {"!connect!", Action::Connect},
            {"send_message", Action::SendMessage},
            {"create_chat", Action::CreateChat},
            {"accept_create_chat", Action::AcceptCreateChat}
        };

        auto it = actionMap.find(actionStr);

        if (it != actionMap.end())
        {
            return it->second;
        }
        else if (actionStr.substr(0, CREATE_CHAT_PREFIX_LENGTH) == "create_chat:")
        {
            return Action::CreateChat;
        }

        return Action::Unknown;
    }
}