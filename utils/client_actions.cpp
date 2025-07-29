#include "client_actions.hpp"
#include <unordered_map>

namespace Utils
{
    const std::unordered_map<std::string, Action> actionMap = {
        {"!connect!", Action::Connect},
        {"register", Action::Register},
        {"authorize", Action::Authorize},
        {"already_authorized", Action::AlreadyAuthorized},
        {"change_name", Action::ChangeName},
        {"new_name", Action::NewClientName},
        {"send_message", Action::SendMessage},
        {"incoming_message", Action::IncomingMessage},
        {"create_chat", Action::CreateChat},
        {"accept_create_chat", Action::AcceptCreateChat},
        {"decline_create_chat", Action::DeclineCreateChat},
        {"new_chat", Action::NewChat},
        {"all_chats", Action::AllChats},
        {"client_chats", Action::ClientChats},
        {"invites", Action::Invites},
        {"clients_by_name", Action::ClientsByName}
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

    std::string actionToString(Action action)
    {
        for (auto it = actionMap.begin(); it != actionMap.end(); it++)
        {
            if (it->second == action)
            {
                return it->first;
            }
        }

        return "";
    }
}