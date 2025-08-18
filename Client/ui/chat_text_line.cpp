#include "chat_text_line.hpp"
#include <QKeyEvent>

UI::ChatTextLine::ChatTextLine(int maxWidth, int height, QWidget* parent) :
	QTextEdit(parent), m_maxWidth(maxWidth), m_height(height)
{
	setMaximumWidth(m_maxWidth);
	setMaximumHeight(m_height);

	setContentsMargins(3, 3, 3, 3);
	connect(this, &QTextEdit::textChanged, this, &ChatTextLine::AdjustHeight);
}

void UI::ChatTextLine::AdjustHeight()
{
	int lines = document()->lineCount();
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