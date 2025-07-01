#pragma once

#include <QComboBox>

namespace UI
{
	class PopUpSingalEmittingQComboBox : public QComboBox
	{
		Q_OBJECT

	public:
		explicit PopUpSingalEmittingQComboBox(QWidget* parent = nullptr);

		void showPopup();

	signals:
		void PoppedUp();
	};
}