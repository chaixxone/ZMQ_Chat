#pragma once

#include <QWidget>
#include <QMessageBox>
#include <QString>

#include <notifiable_interface.hpp>
#include <message_view.hpp>

namespace UI
{
	struct ChatInviteData
	{
		int NotificationID;
		int ChatId;
		bool IsAccepted;
	};

	class ChatInvite : public INotifiable
	{		
	public:
		explicit ChatInvite(const MessageView& messageView, QWidget* parent = nullptr);

		void OnClick() override;

	private:
		QString _author;
		int _chatId;
		int _notificationID;
	};
}