#include <message_item_widget_wrapper.hpp>

#include <QHBoxLayout>

using namespace UI;

MessageItemWidgetWrapper::MessageItemWidgetWrapper(Message* message, bool isCurrentClient, QWidget* parent) :
	QWidget(parent),
	_message(message)
{
	auto layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	if (isCurrentClient)
	{
		layout->addStretch();
		layout->addWidget(_message, 0, Qt::AlignRight);
		_message->setObjectName("current_user");
	}
	else
	{
		layout->addWidget(_message, 0, Qt::AlignLeft);
		layout->addStretch();
		_message->setObjectName("other_user");
	}

	setContentsMargins(0, 0, 0, 0);
}

Message* MessageItemWidgetWrapper::GetMessage() const { return _message; }