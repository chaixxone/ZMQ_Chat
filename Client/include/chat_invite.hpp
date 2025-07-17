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
		int ChatId;
		bool IsAccepted;
	};

	Q_DECLARE_METATYPE(ChatInviteData*);

	class ChatInvite : public INotifiable
	{		
	public:
		explicit ChatInvite(MessageView& messageView, QWidget* parent = nullptr);

		void OnClick() override;

	private:
		QString _author;
		int _chatId;
	};
}