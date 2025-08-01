#pragma once

#include <QWidget>
#include <QMessageBox>
#include <QString>
#include <nlohmann/json.hpp>

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
		explicit ChatInvite(const nlohmann::json& notificationPayload, QWidget* parent = nullptr);

		void OnClick() override;

	private:
		QString _author;
		int _chatId;
		int _notificationID;
	};
}