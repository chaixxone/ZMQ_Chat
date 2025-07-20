#include <message.hpp>

using namespace UI;

Message::Message(size_t id, QString author, QString text, QWidget* parent) :
	QWidget(parent), _id(id), _author(std::move(author)), _content(new QTextEdit(std::move(text)))
{
	_content->setReadOnly(true);
	_content->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_content->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_content->setStyleSheet("border: none; background: transparent;");
	_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	auto layout = new QVBoxLayout;
	layout->addWidget(new QLabel(_author));
	layout->addWidget(_content);
	setLayout(layout);

	QTextDocument* doc = _content->document();
	int documentWidth = _content->viewport()->width();
	doc->setTextWidth(documentWidth);
	int documentHeight = doc->size().height();
	_content->setFixedHeight(documentHeight);
	adjustSize();
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