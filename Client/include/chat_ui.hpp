#pragma once

#include <QtWidgets>

class ChatUI : public QWidget
{
	Q_OBJECT

public:
	explicit ChatUI(QWidget* parent = nullptr);
	~ChatUI();

private:
	QStackedWidget* _pages;
};