#pragma once
#include <QTextEdit>

namespace UI
{
	class ChatTextLine final : public QTextEdit
	{
		Q_OBJECT

	public:
		explicit ChatTextLine(int maxWidth, int minHeight, QWidget* parent = nullptr);

	protected:
		void keyPressEvent(QKeyEvent* event) override;

	private:
		int m_maxWidth;
		int m_minHeight;
		static const int m_maxVisibleLines = 5;

	signals:
		void SendedText(const QString& text);

	private slots:
		void AdjustHeight();
	};
}