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

	auto messageHeadersLayout = new QHBoxLayout;
	auto authorLabel = new QLabel(_author);
	authorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	authorLabel->adjustSize();
	authorLabel->setObjectName("Author Label");
	messageHeadersLayout->addWidget(authorLabel);

	auto mainLayout = new QVBoxLayout;
	mainLayout->addLayout(messageHeadersLayout);
	mainLayout->addWidget(_content);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

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

QSize Message::sizeHint() const
{
	const int infoMargin = 20;

	QTextDocument* doc = _content->document();
	auto firstInfoHeader = findChild<QLabel*>("Author Label");

	int headersHeight = firstInfoHeader->height();
	int contentWidth = _content->viewport()->width();
	int documentHeight = doc->size().height();
	int totalHeight = documentHeight + headersHeight + infoMargin;

	return QSize{ contentWidth, totalHeight };
}