#pragma once
#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QToolButton>
#include <QParallelAnimationGroup>
#include <nlohmann/json.hpp>

#include <message_view.hpp>

namespace UI
{
	class NoticeBox : public QWidget
	{
		Q_OBJECT

	public:
		explicit NoticeBox(const QString& title, QWidget* parent = nullptr);

		void SetupLayout(QLayout* layout);

		void ProcessNotification(const MessageView& messageView);
		void ProcessAllNotifications(const MessageView& messageView);

	signals:
		void InvitationProcessed(int notificationID, int chatId, bool isAccepted);

	private:
		QListWidget* _notices;
		QLabel* _noticeCount;
		QScrollArea* _scrollArea;
		QToolButton* _triangleToolButton;
		QParallelAnimationGroup* _animation;
		int _animationDuration = 250;

		void SetNoticeCountLabel();
		void ProcessNotificationIteration(Utils::Action notificationType, const nlohmann::json& notificationPayload);
	};
}