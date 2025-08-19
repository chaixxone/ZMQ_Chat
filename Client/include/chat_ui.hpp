#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLayout>

#include <client.hpp>
#include <qt_message_observer.hpp>
#include <chat_text_frame.hpp>
#include <chat_text_line.hpp>
#include <qt_helper_window.hpp>
#include <popup_signal_emitting_q_combo_box.hpp>
#include <notice_box.hpp>

namespace UI
{
	class ChatUI : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit ChatUI(std::shared_ptr<Client> client, std::shared_ptr<QtMessageObserver> observer, QWidget* parent = nullptr);
		~ChatUI();

	private:
		QStackedWidget* _pages;
		QWidget* _registerPage;
		QWidget* _loginPage;
		QWidget* _mainPage;
		NoticeBox* _noticeBox;

		ChatTextFrame* _chat;
		ChatTextLine* _messageTextBar;

		QLineEdit* _nameLineEdit;
		PopUpSignalEmittingQComboBox* _userChatIdComboBox;

		QPushButton* _createChatPushButton;
		HelperWindow* _createChatHelperWindow;

		std::shared_ptr<Client> _client;
		std::shared_ptr<QtMessageObserver> _messageObserver;
		int _lineEditsMaxWidth = 300;

		void SetupRegisterPage();
		void OnRegisterAction(const MessageView& message);

		void SetupLoginPage();
		void OnAuthorizeAction(const MessageView& message);

		void SetupMainPage();

		QLayout* SetupSidePanel();

		void ConnectSignalsCreateChat();

		void ConnectSignalsUserChats();

		void ConnectAllSignals();

		void ConnectChatMessageSignals();

		void ConnectNoticeBoxSignals();
	};
}