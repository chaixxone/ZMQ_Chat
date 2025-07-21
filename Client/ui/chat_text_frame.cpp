#include <chat_text_frame.hpp>

using namespace UI;

Q_DECLARE_METATYPE(Message*)

ChatTextFrame::ChatTextFrame(QWidget* parent) : QWidget(parent), _messages(new QListWidget), _currentChat(-1)
{
	_messages->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	auto vMessagesLayout = new QVBoxLayout;
	vMessagesLayout->addWidget(_messages);
	vMessagesLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(vMessagesLayout);
}

void ChatTextFrame::AddMessage(Message* message)
{
	int contentWidth = _messages->viewport()->width();
	message->setFixedWidth(contentWidth);
	auto messageItem = new QListWidgetItem(_messages);
	_messages->setItemWidget(messageItem, message);
	messageItem->setSizeHint(message->sizeHint());
}

void ChatTextFrame::RemoveMessage(int messageId)
{
	size_t messages = _messages->count();
	size_t left = 0;
	size_t right = messages - 1;
	size_t index = 0;

	while (left <= right)
	{
		size_t middle = (left + right) / 2;
		size_t messageIdAtMiddle = qvariant_cast<Message*>(_messages->item(static_cast<int>(middle))->data(Qt::UserRole))->GetId();

		if (messageId < messageIdAtMiddle)
		{
			right = middle - 1;
		}
		else if (messageId > messageIdAtMiddle)
		{
			left = middle + 1;
		}
		else
		{
			index = middle;
			break;
		}
	}

	_messages->removeItemWidget(_messages->item(static_cast<int>(index)));
}

ChatTextFrame::~ChatTextFrame() {}

void ChatTextFrame::SetCurrentChat(const QString& chatIdStr) noexcept
{
	_currentChat = chatIdStr.toInt();
}

int ChatTextFrame::GetCurrentChat() const noexcept
{
	return _currentChat;
}

void ChatTextFrame::RemoveMessages()
{
	_messages->clear();
}