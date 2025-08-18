#pragma once
#include <QTextEdit>

namespace UI
{
	class ChatTextLine final : public QTextEdit
	{
		Q_OBJECT
		Q_PROPERTY(QColor cursorColor READ GetCursorColor WRITE SetCursorColor)

	public:
		explicit ChatTextLine(int maxWidth, int height, QWidget* parent = nullptr);

		QColor GetCursorColor() const;
		void SetCursorColor(QColor color);

	protected:
		void keyPressEvent(QKeyEvent* event) override;
		void paintEvent(QPaintEvent* event) override;

	private:		
		QColor m_cursorColor;
		int m_maxWidth;
		int m_height;
		static const int m_maxVisibleLines = 5;

	signals:
		void SendedText(const QString& text);

	private slots:
		void AdjustHeight();
	};
}