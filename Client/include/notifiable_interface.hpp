#pragma once

#include <QWidget>

enum class Notifications
{
	ChatInvite,
	MessageReply,
	Emote
};

class INotifiable : public QWidget
{
	Q_OBJECT

public:
	explicit INotifiable(QWidget* parent = nullptr);
	virtual ~INotifiable();
	virtual void OnClick() = 0;

signals:
	void NotificationProcessed(Notifications notificationType, QVariant data);
	void NotificationWatched(Notifications notificationType);
};