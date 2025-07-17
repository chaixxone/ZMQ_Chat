#pragma once
#include <QWidget>
#include <QListWidget>
#include <QScrollArea>
#include <QToolButton>
#include <QParallelAnimationGroup>

namespace UI
{
	class NoticeBox : public QWidget
	{
	public:
		explicit NoticeBox(const QString& title, QWidget* parent = nullptr);

		void SetupLayout(QLayout* layout);

	private:
		QListWidget* _notices;
		QScrollArea* _scrollArea;
		QToolButton* _triangleToolButton;
		QParallelAnimationGroup* _animation;
		int _animationDuration = 250;
	};
}