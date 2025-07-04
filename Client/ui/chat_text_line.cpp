#include "chat_text_line.hpp"
#include <QKeyEvent>

UI::ChatTextLine::ChatTextLine(int maxWidth, int minHeight, QWidget* parent) :
	QTextEdit(parent), m_maxWidth(maxWidth), m_minHeight(minHeight)
{
	resize(m_maxWidth, m_minHeight);
	setContentsMargins(3, 3, 3, 3);
	connect(this, &QTextEdit::textChanged, this, &ChatTextLine::AdjustHeight);
}

void UI::ChatTextLine::AdjustHeight()
{
	int lines = document()->lineCount();
	int heightMultiplier = qMin(lines, m_maxVisibleLines);
	resize(m_maxWidth, m_minHeight * heightMultiplier);
}

void UI::ChatTextLine::keyPressEvent(QKeyEvent* event)
{
	QTextEdit::keyPressEvent(event);

	if (event->key() == Qt::Key::Key_Return && (event->modifiers() & Qt::KeyboardModifier::ControlModifier))
	{
		qDebug() << "ctrl + enter";

		QString text = document()->toPlainText().trimmed();

		if (!text.isEmpty())
		{
			document()->clear();
			emit SendedText(text);
			return;
		}
	}
}