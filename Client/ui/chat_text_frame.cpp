#include <chat_text_frame.hpp>

using namespace UI;

Q_DECLARE_METATYPE(Message*)

ChatTextFrame::ChatTextFrame(QWidget* parent) : QWidget(parent), _messages(new QListWidget)
{
	auto vMessagesLayout = new QVBoxLayout;
	vMessagesLayout->addWidget(_messages);
	setLayout(vMessagesLayout);
}

void ChatTextFrame::AddMessage(Message* message)
{
	auto messageItem = new QListWidgetItem;
	QVariant messageData = QVariant::fromValue(message);
	messageItem->setData(Qt::UserRole, messageData);
	_messages->addItem(messageItem);
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