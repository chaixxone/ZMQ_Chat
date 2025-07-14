#include <popup_signal_emitting_q_combo_box.hpp>

UI::PopUpSignalEmittingQComboBox::PopUpSignalEmittingQComboBox(QWidget* parent) :
	QComboBox(parent),
	_popupDelayTimer(new QTimer(this))
{
	_popupDelayTimer->setSingleShot(true);
	_popupDelayTimer->setInterval(std::chrono::milliseconds(50));	
	connect(this, &PopUpSignalEmittingQComboBox::PoppedUp, _popupDelayTimer, QOverload<>::of(&QTimer::start));
	connect(_popupDelayTimer, &QTimer::timeout, this, [this]() {
		QComboBox::showPopup();
	});
}

void UI::PopUpSignalEmittingQComboBox::showPopup()
{
	emit PoppedUp();
}