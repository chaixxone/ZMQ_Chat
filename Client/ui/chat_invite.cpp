#include "chat_invite.hpp"
#include <QLabel>
#include <nlohmann/json.hpp>

using namespace UI;

Q_DECLARE_METATYPE(ChatInviteData*);

ChatInvite::ChatInvite(const nlohmann::json& notificationPayload, QWidget* parent) :
	INotifiable(parent)
{
	_author         = QString::fromStdString(notificationPayload["author"].get<std::string>());
	_chatId         = notificationPayload["chat_id"].get<int>();
	_notificationID = notificationPayload["notification_id"].get<int>();

	auto label = new QLabel("chat invite", this);
}

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
			emit NotificationProcessed(Notifications::ChatInvite, QVariant::fromValue(ChatInviteData{ _notificationID, _chatId, true }));
		}
		else if (pressedButtonRole == QMessageBox::ButtonRole::NoRole)
		{
			emit NotificationProcessed(Notifications::ChatInvite, QVariant::fromValue(ChatInviteData{ _notificationID, _chatId, false }));
		}

		emit NotificationWatched(Notifications::ChatInvite);

		askWindow->deleteLater();
	});

	askWindow->show();
}