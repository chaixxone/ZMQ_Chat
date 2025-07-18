#pragma once

#include <QtWidgets>
#include <client.hpp>
#include <qt_message_observer.hpp>
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
		std::shared_ptr<Client> _client;
		std::shared_ptr<QtMessageObserver> _messageObserver;
	};
}