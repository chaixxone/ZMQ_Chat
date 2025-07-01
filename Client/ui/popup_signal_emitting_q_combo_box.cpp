#include <popup_signal_emitting_q_combo_box.hpp>

UI::PopUpSingalEmittingQComboBox::PopUpSingalEmittingQComboBox(QWidget* parent) : QComboBox(parent) {}

void UI::PopUpSingalEmittingQComboBox::showPopup()
{
	emit PoppedUp();
	QComboBox::showPopup();
}