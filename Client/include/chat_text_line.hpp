#pragma once
#include <QTextEdit>

class ChatTextLine final : public QTextEdit
{
	Q_OBJECT

public:
	explicit ChatTextLine(int maxWidth, int minHeight, QWidget* parent = nullptr);

private:
	int m_maxWidth;
	int m_minHeight;
	static const int m_maxVisibleLines = 5;

private slots:
	void AdjustHeight();
};