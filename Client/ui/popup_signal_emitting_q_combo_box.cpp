#include <popup_signal_emitting_q_combo_box.hpp>

UI::PopUpSignalEmittingQComboBox::PopUpSignalEmittingQComboBox(QWidget* parent) : QComboBox(parent) {}

void UI::PopUpSignalEmittingQComboBox::showPopup()
{
	emit PoppedUp();
	QComboBox::showPopup();
}