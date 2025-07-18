#pragma once
#include <QWidget>
#include <QListWidget>
#include <QScrollArea>
#include <QToolButton>
#include <QParallelAnimationGroup>
#include <message_view.hpp>

namespace UI
{
	class NoticeBox : public QWidget
	{
		Q_OBJECT

	public:
		explicit NoticeBox(const QString& title, QWidget* parent = nullptr);

		void SetupLayout(QLayout* layout);

		void AddNotification(const MessageView& messageView);

	signals:
		void InvitationProcessed(int chatId, bool isAccepted);

	private:
		QListWidget* _notices;
		QScrollArea* _scrollArea;
		QToolButton* _triangleToolButton;
		QParallelAnimationGroup* _animation;
		int _animationDuration = 250;

		void RemoveNotification(QListWidgetItem* item);
	};
}