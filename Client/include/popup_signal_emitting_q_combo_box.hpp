#pragma once

#include <QComboBox>
#include <QTimer>

namespace UI
{
	class PopUpSignalEmittingQComboBox : public QComboBox
	{
		Q_OBJECT

	public:
		explicit PopUpSignalEmittingQComboBox(QWidget* parent = nullptr);

		void showPopup();

	signals:
		void PoppedUp();

	private:
		QTimer* _popupDelayTimer;
	};
}