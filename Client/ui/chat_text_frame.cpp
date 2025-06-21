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

ChatTextFrame::~ChatTextFrame() {}