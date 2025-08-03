#include <chat_text_frame.hpp>
#include <message_item_widget_wrapper.hpp>

using namespace UI;

Q_DECLARE_METATYPE(Message*)

ChatTextFrame::ChatTextFrame(QWidget* parent) : QWidget(parent), _messages(new QListWidget), _currentChat(-1)
{
	const int spaceBetweenItems = 5;
	_messages->setSpacing(spaceBetweenItems);
	_messages->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	_messages->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

	auto vMessagesLayout = new QVBoxLayout;
	vMessagesLayout->addWidget(_messages);
	vMessagesLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(vMessagesLayout);
}

void ChatTextFrame::AddMessage(Message* message, bool isCurrentClient)
{
	const double messageWidgetWidthRatio = 0.6;

	int contentWidth = _messages->viewport()->width();
	message->setFixedWidth(contentWidth * messageWidgetWidthRatio);

	auto messageWrapper = new MessageItemWidgetWrapper(message, isCurrentClient);

	auto messageItem = new QListWidgetItem(_messages);
	_messages->setItemWidget(messageItem, messageWrapper);
	messageItem->setSizeHint(message->sizeHint());

	const int maximumShowDifference = 50;
	QScrollBar* scrollBar = _messages->verticalScrollBar();

	if (scrollBar->maximum() - scrollBar->value() <= maximumShowDifference + _spaceBetweenItems)
	{
		_messages->scrollToBottom();
	}
}

void ChatTextFrame::RemoveMessage(size_t messageId)
{
	int left = 0;
	int right = _messages->count() - 1;
	int index = 0;

	while (left <= right)
	{
		int middle = (left + right) / 2;
		QListWidgetItem* middleItem = _messages->item(middle);
		auto messageWrapper = static_cast<MessageItemWidgetWrapper*>(_messages->itemWidget(middleItem));
		size_t messageIdAtMiddle = messageWrapper->GetMessage()->GetId();

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

	delete _messages->takeItem(index);
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