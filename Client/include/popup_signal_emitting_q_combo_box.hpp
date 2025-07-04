#pragma once

#include <QComboBox>

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
	};
}