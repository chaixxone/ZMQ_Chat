#pragma once

#include <QWidget>

#include <message.hpp>

namespace UI
{
	class MessageItemWidgetWrapper : public QWidget
	{
	public:
		explicit MessageItemWidgetWrapper(Message* message, QWidget* parent = nullptr);

		~MessageItemWidgetWrapper();

		Message* GetMessage() const;

		void AlignRight();

	private:
		Message* _message;
	};
}