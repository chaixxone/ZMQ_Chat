#include "qt_helper_window.hpp"
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <chrono>
#include <qdebug.h>

using namespace std::chrono;

using namespace UI;

HelperWindow::HelperWindow(QWidget* parent) :
	QWidget(parent, Qt::Tool), 
	m_lineEdit(new QLineEdit), 
	m_confirmButton(new QPushButton("Confirm")), 
	m_listWidget(new QListWidget),
	m_chosenItemsListWidget(new QListWidget)
{
	connect(m_confirmButton, &QPushButton::clicked, this, &HelperWindow::OnConfirmClicked);
	connect(m_lineEdit, &QLineEdit::textChanged, this, &HelperWindow::OnTextChanged);
	connect(m_listWidget, &QListWidget::itemClicked, m_chosenItemsListWidget, [this](QListWidgetItem* item) {
		if (m_chosenItemsListWidget->findItems(item->text(), Qt::MatchContains).empty())
		{
			m_chosenItemsListWidget->addItem(item->text());
		}
	});
	connect(m_chosenItemsListWidget, &QListWidget::itemClicked, m_chosenItemsListWidget, [this](QListWidgetItem* item) {
		int itemIndex = m_chosenItemsListWidget->indexFromItem(item).row();
		QListWidgetItem* itemToRemove = m_chosenItemsListWidget->takeItem(itemIndex);
		delete itemToRemove;
	});

	auto vLayout = new QVBoxLayout;
	vLayout->addWidget(m_lineEdit);
	vLayout->addWidget(m_confirmButton);
	vLayout->addWidget(m_listWidget);
	vLayout->addWidget(m_chosenItemsListWidget);
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

bool HelperWindow::IsHidden() const
{
	return isHidden();
}

const std::string HelperWindow::GetChosenClientsString() const
{
	std::stringstream namesStream;

	for (const auto& name : m_chosenItemsListWidget->findItems("*", Qt::MatchWildcard))
	{
		namesStream << name->text().toStdString() << ' ';
	}

	std::string namesString = namesStream.str();

	if (!namesString.empty())
	{
		namesString.pop_back();
	}

	return namesString;
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