#pragma once

#include <QtWidgets>

namespace UI
{
	class Message : public QWidget
	{
	public:
		explicit Message(size_t id, QString author, QString text, QWidget* parent = nullptr);
		~Message();

		size_t GetId() const noexcept;

		QString GetContent() const noexcept;

		QSize sizeHint() const override;

	public slots:
		void EditText();

	private:
		const size_t _id;
		QString _author;
		QTextEdit* _content;
	};
}