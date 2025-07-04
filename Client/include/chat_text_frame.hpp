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
		void SetCurrentChat(int chatId) noexcept;

	private:
		QListWidget* _messages;
		int _currentChat;

	private slots:
		void RemoveMessage(int messageId);
	};
}