#pragma once

#include <QtWidgets>

namespace UI
{
	class Message : public QWidget
	{
	public:
		explicit Message(QWidget* parent = nullptr);

		size_t GetId() const noexcept
		{
			return _id;
		}

	public slots:
		void EditText();

	private:
		const size_t _id;
		QString _author;
		QTextBlock* _content;
	};
}