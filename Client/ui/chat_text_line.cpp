#include "chat_text_line.hpp"
#include <QKeyEvent>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <logger.hpp>

UI::ChatTextLine::ChatTextLine(int maxWidth, int height, QWidget* parent) :
	QTextEdit(parent), 
	m_cursorBlinkTimer(new QTimer(this)), 
	m_maxWidth(maxWidth), 
	m_height(height), 
	m_isCursorHidden(false)
{
	setCursorWidth(0);

	setMaximumWidth(m_maxWidth);
	setMaximumHeight(m_height);
	setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

	setContentsMargins(3, 3, 3, 3);
	connect(this, &QTextEdit::textChanged, this, &ChatTextLine::AdjustHeight);

	m_cursorBlinkTimer->setInterval(m_cursorBlinkDuration);
	connect(m_cursorBlinkTimer, &QTimer::timeout, this, [this]() { m_isCursorHidden = !m_isCursorHidden; });
	m_cursorBlinkTimer->start();
}

void UI::ChatTextLine::AdjustHeight()
{
	QFontMetrics metrics{ viewport()->font() };
	int fontHeight = metrics.height();

	int documentHeight = document()->documentLayout()->documentSize().height();
	int lines = documentHeight / fontHeight;
	Logger::LogDebug(std::to_string(lines), "lines");

	int heightMultiplier = qMin(lines, m_maxVisibleLines);
	setMaximumHeight(m_height * heightMultiplier);
}

void UI::ChatTextLine::keyPressEvent(QKeyEvent* event)
{
	QTextEdit::keyPressEvent(event);

	if (event->key() == Qt::Key::Key_Return && (event->modifiers() & Qt::KeyboardModifier::ControlModifier))
	{
		QString text = document()->toPlainText().trimmed();

		if (!text.isEmpty())
		{
			document()->clear();
			emit SendedText(text);
			return;
		}
	}
}

QColor UI::ChatTextLine::GetCursorColor() const
{
	return m_cursorColor;
}

void UI::ChatTextLine::SetCursorColor(QColor color)
{
	m_cursorColor = color;
}

void UI::ChatTextLine::paintEvent(QPaintEvent* event)
{
	QTextEdit::paintEvent(event);	

	if (textCursor().hasSelection())
	{
		return;
	}

	if (m_isCursorHidden || !hasFocus())
	{
		return;
	}

	QPainter painter{ viewport() };
	QPen pen = painter.pen();
	pen.setColor(m_cursorColor);
	painter.setPen(pen);
	painter.drawRect(cursorRect());
}