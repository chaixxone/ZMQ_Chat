#include <message_reply.hpp>
#include <QLabel>

using namespace UI;

Q_DECLARE_METATYPE(MessageReplyData*);

MessageReply::MessageReply(const nlohmann::json& notificationPayload, QWidget* parent) : 
	INotifiable(parent)	
{
	_author			= QString::fromStdString(notificationPayload["author"].get<std::string>());
	_messageID		= notificationPayload["replied_message_id"].get<size_t>();
	_chatID			= notificationPayload["chat_id"].get<int>();
	_notificationID = notificationPayload["notification_id"].get<int>();
	auto noticeBoxContentLabel = new QLabel(_author + " replied on your message");
}

void MessageReply::OnClick()
{
	MessageReplyData replyData{ std::move(_author), _messageID, _chatID, _notificationID };
	emit NotificationProcessed(Notifications::MessageReply, QVariant::fromValue(replyData));
}

void MessageReply::ReplyChecked()
{
	emit NotificationWatched(Notifications::MessageReply);
}