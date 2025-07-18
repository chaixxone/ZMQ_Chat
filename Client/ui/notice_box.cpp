#include "notice_box.hpp"
#include <chat_invite.hpp>
#include <notifiable_interface.hpp>

#include <QPropertyAnimation>
#include <QVBoxLayout>

using namespace UI;

NoticeBox::NoticeBox(const QString& title, QWidget* parent) :
	QWidget(parent),
	_notices(new QListWidget),
	_scrollArea(new QScrollArea(this)),
	_triangleToolButton(new QToolButton(this)),
	_animation(new QParallelAnimationGroup(this))
{
	_triangleToolButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
	_triangleToolButton->setArrowType(Qt::ArrowType::RightArrow);
	_triangleToolButton->setText(title);
	_triangleToolButton->setCheckable(true);
	_triangleToolButton->setChecked(false);

	_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	_scrollArea->setMinimumHeight(0);
	_scrollArea->setMaximumHeight(0);

	_animation->addAnimation(new QPropertyAnimation(this, "minimumHeight"));
	_animation->addAnimation(new QPropertyAnimation(this, "maximumHeight"));
	_animation->addAnimation(new QPropertyAnimation(_scrollArea, "maximumHeight"));

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(_triangleToolButton);
	mainLayout->addWidget(_scrollArea);
	setLayout(mainLayout);

	connect(_triangleToolButton, &QToolButton::clicked, [this](bool checked) {
		_triangleToolButton->setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
		_animation->setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
		_animation->start();
	});

	auto noticesListLayout = new QVBoxLayout;
	noticesListLayout->setSpacing(0);
	noticesListLayout->setContentsMargins(0, 0, 0, 0);
	noticesListLayout->addWidget(_notices);
	SetupLayout(noticesListLayout);
}

void NoticeBox::SetupLayout(QLayout* layout)
{
	_scrollArea->setLayout(layout);
	int collapsedHeight = sizeHint().height() - _scrollArea->maximumHeight();
	int contentHeight = layout->sizeHint().height();

	for (int i = 0; i < _animation->animationCount() - 1; i++)
	{
		auto spoilerAnimation = static_cast<QPropertyAnimation*>(_animation->animationAt(i));
		spoilerAnimation->setDuration(_animationDuration);
		spoilerAnimation->setStartValue(collapsedHeight);
		spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
	}

	auto contentAnimation = static_cast<QPropertyAnimation*>(_animation->animationAt(_animation->animationCount() - 1));
	contentAnimation->setDuration(_animationDuration);
	contentAnimation->setStartValue(0);
	contentAnimation->setEndValue(contentHeight);
}

void NoticeBox::AddNotification(const MessageView& messageView)
{
	INotifiable* notice = nullptr;

	switch (messageView.Action)
	{
	case Utils::Action::CreateChat:
		notice = new ChatInvite(messageView);
		auto item = new QListWidgetItem(_notices);
		_notices->setItemWidget(item, notice);
		break;
	}

	// TODO: notify user by a red indicator
}