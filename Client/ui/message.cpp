#include <message.hpp>

using namespace UI;

Message::Message(size_t id, QString author, QString text, QWidget* parent) :
	QWidget(parent), _id(id), _author(std::move(author)), _content(new QTextEdit(std::move(text)))
{
	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(_author));
	_content->setReadOnly(true);
	_content->setStyleSheet("border: none; background: transparent;");
	_content->setMinimumHeight(50);
	layout->addWidget(_content);
	setLayout(layout);
}

Message::~Message() {}

size_t Message::GetId() const noexcept
{
	return _id;
}

QString Message::GetContent() const noexcept
{
	return _content->toPlainText();
}