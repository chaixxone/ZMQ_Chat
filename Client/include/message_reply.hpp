#pragma once

#include <notifiable_interface.hpp>
#include <message_view.hpp>

namespace UI
{
	struct MessageReplyData
	{
		QString Author;
		size_t MessageID;
		int ChatID;
	};

	class MessageReply : public INotifiable
	{
	public:
		explicit MessageReply(const MessageView& messageView, QWidget* parent = nullptr);

		void OnClick() override;

	public slots:
		void ReplyChecked();

	private:
		QString _author;
		size_t _messageID;
		int _chatID;
	};
}