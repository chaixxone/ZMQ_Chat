#include <message.hpp>

#include <QFile>

using namespace UI;

Message::Message(size_t id, QString author, QString&& text, QWidget* parent) :
	QWidget(parent), _id(id), _author(std::move(author))
{
	text.replace("\n", "<br>");
	_content = new QTextEdit(std::move(text));

	_content->setReadOnly(true);
	_content->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_content->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	auto messageHeadersLayout = new QHBoxLayout;
	auto authorLabel = new QLabel(_author);
	authorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	authorLabel->adjustSize();
	authorLabel->setObjectName("Author Label");

	QFont biggestFont("Jetbrains Mono", 14);
	int biggestFontSize = biggestFont.pointSize();
	authorLabel->setMaximumHeight(biggestFontSize + _infoMargin);
	messageHeadersLayout->addWidget(authorLabel);

	auto mainLayout = new QVBoxLayout;
	mainLayout->addLayout(messageHeadersLayout);
	mainLayout->addWidget(_content);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	QTextDocument* doc = _content->document();
	int documentWidth = _content->viewport()->width();
	doc->setTextWidth(documentWidth);
	
	QFont defaultFont = doc->defaultFont();
	int defaultFontSize = defaultFont.pointSize();
	int documentHeight = doc->size().height();
	int maximumDocumentHeight = documentHeight + documentHeight * biggestFontSize / defaultFontSize;

	_content->setMaximumHeight(maximumDocumentHeight);
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

void Message::SetRepliedState(Message* repliedMessage)
{
	QLayout* aLayout = layout();
	QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(aLayout);

	QWidget* replyWrapper = new QWidget(this);
	QVBoxLayout* wrapperLayout = new QVBoxLayout;
	wrapperLayout->addWidget(repliedMessage);
	replyWrapper->setLayout(wrapperLayout);	

	mainLayout->insertWidget(0, nullptr, 0);
}

QSize Message::sizeHint() const
{
	QTextDocument* doc = _content->document();
	auto firstInfoHeader = findChild<QLabel*>("Author Label");

	int headersHeight = firstInfoHeader->height();
	int contentWidth = _content->viewport()->width();
	int documentHeight = doc->size().height();
	int totalHeight = documentHeight + headersHeight + _infoMargin;

	return QSize{ contentWidth, totalHeight };
}

void Message::paintEvent(QPaintEvent* event)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter painter{ this };
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

	QWidget::paintEvent(event);
}