#include "chat_invite.hpp"

using namespace UI;

explicit ChatInvite::ChatInvite(MessageView& messageView, QWidget* parent) :
	QWidget(parent),
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

	connect(askWindow, &QMessageBox::buttonClicked, [askWindow, this]() {
		int pressedButtonCode = askWindow->exec();

		if (pressedButtonCode == QMessageBox::StandardButton::Yes)
		{
			emit InvitationAccepted(_chatId);
		}
		else if (pressedButtonCode == QMessageBox::StandardButton::No)
		{
			emit InvitationDeclined(_chatId);
		}

		emit InvitationProcessed();
	});
}