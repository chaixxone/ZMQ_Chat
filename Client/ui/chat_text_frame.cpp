#include <chat_text_frame.hpp>

using namespace UI;

Q_DECLARE_METATYPE(Message*)

ChatTextFrame::ChatTextFrame(QWidget* parent) : QWidget(parent), _messages(new QListWidget)
{
	auto vMessagesLayout = new QVBoxLayout;
	vMessagesLayout->addWidget(_messages);
	vMessagesLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(vMessagesLayout);
}

void ChatTextFrame::AddMessage(Message* message)
{
	auto messageItem = new QListWidgetItem(_messages);
	_messages->setItemWidget(messageItem, message);
	// TODO: adjust message size
	messageItem->setSizeHint(QSize{ 50, 50 });
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
		size_t messageIdAtMiddle = qvariant_cast<Message*>(_messages->item(middle)->data(Qt::UserRole))->GetId();

		if (messageIdAtMiddle < middle)
		{
			right = middle;
		}
		else if (messageId > middle)
		{
			left = middle;
		}
		else
		{
			index = middle;
			break;
		}
	}

	_messages->removeItemWidget(_messages->item(index));
}

ChatTextFrame::~ChatTextFrame() {}