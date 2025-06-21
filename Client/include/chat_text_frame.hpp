#pragma once

#include <QtWidgets>

namespace UI
{
	class ChatTextFrame : public QWidget
	{
		Q_OBJECT

	public:
		explicit ChatTextFrame(QWidget* parent = nullptr);
		~ChatTextFrame();

	private:
		QListWidget* _messages;
	};
}