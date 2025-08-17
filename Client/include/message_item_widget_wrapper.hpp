#pragma once

#include <QWidget>

#include <message.hpp>

namespace UI
{
	class MessageItemWidgetWrapper : public QWidget
	{
		Q_OBJECT

	public:
		explicit MessageItemWidgetWrapper(Message* message, bool isCurrentClient, QWidget* parent = nullptr);

		Message* GetMessage() const;

	private:
		Message* _message;
	};
}