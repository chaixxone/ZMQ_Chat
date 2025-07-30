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
		int GetCurrentChat() const noexcept;
		void RemoveMessages();

	signals:
		void CurrentChatChanged(int currentChat);

	public slots:
		void AddMessage(Message* message);
		void SetCurrentChat(const QString& chatIdStr) noexcept;

	private:
		QListWidget* _messages;
		int _currentChat;

	private slots:
		void RemoveMessage(size_t messageId);
	};
}