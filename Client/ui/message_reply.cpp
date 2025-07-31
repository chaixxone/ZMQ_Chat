#include <message_reply.hpp>
#include <QLabel>

using namespace UI;

Q_DECLARE_METATYPE(MessageReplyData*);

MessageReply::MessageReply(const MessageView& messageView, QWidget* parent) : 
	INotifiable(parent),
	_author(QString::fromStdString(messageView.Author)),
	_messageID(messageView.ID.value()),
	_chatID(messageView.ChatID)
{
	auto noticeBoxContentLabel = new QLabel(_author + " replied on your message");
}

void MessageReply::OnClick()
{
	MessageReplyData replyData{ std::move(_author), _messageID, _chatID };
	emit NotificationProcessed(Notifications::MessageReply, QVariant::fromValue(replyData));
}

void MessageReply::ReplyChecked()
{
	emit NotificationWatched(Notifications::MessageReply);
}