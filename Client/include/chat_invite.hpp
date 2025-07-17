#pragma once

#include <QWidget>
#include <QMessageBox>
#include <QString>

#include <notifiable_interface.hpp>
#include <message_view.hpp>

namespace UI
{
	class ChatInvite : public QWidget, public INotifiable
	{
		Q_OBJECT

	public:
		explicit ChatInvite(MessageView& messageView, QWidget* parent = nullptr);

		void OnClick() override;

	signals:
		void InvitationAccepted(int chatId);

		void InvitationDeclined(int chatId);

		void InvitationProcessed();

	private:
		QString _author;
		int _chatId;
	};
}