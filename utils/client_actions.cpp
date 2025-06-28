#include "client_actions.hpp"
#include <unordered_map>

namespace Utils
{
    const short CREATE_CHAT_PREFIX_LENGTH = 12;

    const std::unordered_map<std::string, Action> actionMap = {
        {"!connect!", Action::Connect},
        {"send_message", Action::SendMessage},
        {"change_name", Action::ChangeName},
        {"create_chat", Action::CreateChat},
        {"accept_create_chat", Action::AcceptCreateChat}
    };

    Action stringToAction(const std::string& actionStr)
    {      
        auto it = actionMap.find(actionStr);

        if (it != actionMap.end())
        {
            return it->second;
        }

        return Action::Unknown;
    }
}