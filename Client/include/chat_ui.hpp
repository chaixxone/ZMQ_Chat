#pragma once

#include <QtWidgets>
#include <client.hpp>
#include <qt_message_observer.hpp>

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
		QWidget* _loginPage;
		QWidget* _mainPage;
		std::shared_ptr<Client> _client;
		std::shared_ptr<QtMessageObserver> _messageObserver;
	};
}