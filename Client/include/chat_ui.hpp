#pragma once

#include <QtWidgets>

namespace UI
{
	class ChatUI : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit ChatUI(QWidget* parent = nullptr);
		~ChatUI();

	private:
		QStackedWidget* _pages;
		QWidget* _loginPage;
		QWidget* _mainPage;
	};
}