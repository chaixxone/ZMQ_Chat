#include "chat_invite.hpp"

using namespace UI;

ChatInvite::ChatInvite(const MessageView& messageView, QWidget* parent) :
	INotifiable(parent),
	_author(QString::fromStdString(messageView.Author)),
	_chatId(messageView.ChatID) {}

void ChatInvite::OnClick()
{
	QString windowTitle = "Chat invite";
	QString messageBoxText = "Do you want to join chat created by " + _author + "?";
	auto askWindow = new QMessageBox(
		QMessageBox::Icon::NoIcon,
		windowTitle,
		messageBoxText,
		QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
		parentWidget()
	);

	connect(askWindow, &QMessageBox::buttonClicked, [askWindow, this](QAbstractButton* button) {
		QMessageBox::ButtonRole pressedButtonRole = askWindow->buttonRole(button);

		if (pressedButtonRole == QMessageBox::ButtonRole::YesRole)
		{
			emit NotificationProcessed(Notifications::ChatInvite, QVariant::fromValue(ChatInviteData{ _chatId, true }));
		}
		else if (pressedButtonRole == QMessageBox::ButtonRole::NoRole)
		{
			emit NotificationProcessed(Notifications::ChatInvite, QVariant::fromValue(ChatInviteData{ _chatId, false }));
		}

		emit NotificationWatched(Notifications::ChatInvite);

		askWindow->deleteLater();
	});
}