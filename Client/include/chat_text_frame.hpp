#pragma once

#include <QtWidgets>

#include <message.hpp>

namespace UI
{
	class ChatTextFrame : public QWidget
	{
		Q_OBJECT

	public:
		explicit ChatTextFrame(QWidget* parent = nullptr);
		~ChatTextFrame();

	public slots:
		void AddMessage(Message* message);

	private:
		QListWidget* _messages;

	private slots:
		void RemoveMessage(int messageId);
	};
}