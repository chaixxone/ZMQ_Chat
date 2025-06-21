#include "chat_text_line.hpp"

ChatTextLine::ChatTextLine(int maxWidth, int minHeight, QWidget* parent) :
	QTextEdit(parent), m_maxWidth(maxWidth), m_minHeight(minHeight)
{
	resize(m_maxWidth, m_minHeight);
	setContentsMargins(3, 3, 3, 3);
	connect(this, &QTextEdit::textChanged, this, &ChatTextLine::AdjustHeight);
}

void ChatTextLine::AdjustHeight()
{
	int lines = document()->lineCount();
	int heightMultiplier = qMin(lines, m_maxVisibleLines);
	resize(m_maxWidth, m_minHeight * heightMultiplier);
}