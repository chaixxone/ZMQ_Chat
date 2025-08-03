#include <message_item_widget_wrapper.hpp>

#include <QHBoxLayout>

using namespace UI;

MessageItemWidgetWrapper::MessageItemWidgetWrapper(Message* message, QWidget* parent) :
	QWidget(parent),
	_message(message)
{
	setContentsMargins(0, 0, 0, 0);
}

MessageItemWidgetWrapper::~MessageItemWidgetWrapper()
{
	if (_message && _message->parent() == nullptr)
	{
		delete _message;
	}
}

Message* MessageItemWidgetWrapper::GetMessage() const { return _message; }

void MessageItemWidgetWrapper::AlignRight()
{
	auto layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addStretch();
	layout->addWidget(_message, 0, Qt::AlignRight);
}