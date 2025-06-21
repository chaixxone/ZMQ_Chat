#include <chat_text_frame.hpp>

using namespace UI;

Q_DECLARE_METATYPE(QListWidgetItem*)

ChatTextFrame::ChatTextFrame(QWidget* parent) : QWidget(parent), _messages(new QListWidget)
{
	auto vMessagesLayout = new QVBoxLayout;
	vMessagesLayout->addWidget(_messages);
	setLayout(vMessagesLayout);
}

ChatTextFrame::~ChatTextFrame() {}