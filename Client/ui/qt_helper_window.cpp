#include "qt_helper_window.hpp"
#include <QVBoxLayout>
#include <chrono>
#include <qdebug.h>

using namespace std::chrono;

using namespace UI;

HelperWindow::HelperWindow(QWidget* parent) :
	QWidget(parent, Qt::Tool), 
	m_lineEdit(new QLineEdit), 
	m_confirmButton(new QPushButton("Confirm")), 
	m_listWidget(new QListWidget)
{
	connect(m_confirmButton, &QPushButton::clicked, this, &HelperWindow::OnConfirmClicked);
	connect(m_lineEdit, &QLineEdit::textChanged, this, &HelperWindow::OnTextChanged);

	auto vLayout = new QVBoxLayout;
	vLayout->addWidget(m_lineEdit);
	vLayout->addWidget(m_confirmButton);
	vLayout->addWidget(m_listWidget);
	vLayout->addStretch(0);
	m_listWidget->hide();

	setLayout(vLayout);
	setFixedSize(400, 300);
}

HelperWindow::~HelperWindow() {}

void HelperWindow::SetPlaceholderTextLineEdit(const QString& text)
{
	m_lineEdit->setPlaceholderText(text);
}

void HelperWindow::OnConfirmClicked()
{
	emit ConfirmClicked();
}

void HelperWindow::hideEvent(QHideEvent* event)
{
	m_lineEdit->clear();
	QWidget::hideEvent(event);
}

void HelperWindow::OnTextChanged(const QString& text)
{
	emit TextChanged(text);
}

void HelperWindow::AddItems(const QStringList& list)
{
	auto start = steady_clock::now();
	m_listWidget->clear();
	m_listWidget->addItems(list);
	auto end = steady_clock::now();
	long long elapsed = duration_cast<milliseconds>(end - start).count();
	qDebug() << "'Swapped' items in HelperWindow::m_listWidget by " << elapsed << " millisec";
}

void HelperWindow::HideClientList()
{
	m_listWidget->hide();
}

void HelperWindow::ShowClientList()
{
	m_listWidget->show();
}