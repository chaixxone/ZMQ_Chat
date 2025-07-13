#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>

namespace UI
{
	class HelperWindow : public QWidget
	{
		Q_OBJECT

	public:
		explicit HelperWindow(QWidget* parent = nullptr);
		~HelperWindow();

		void SetPlaceholderTextLineEdit(const QString& text);

	signals:
		void ConfirmClicked();

		void TextChanged(const QString&);

	protected:
		void hideEvent(QHideEvent* event) override;

	private:
		QLineEdit* m_lineEdit;
		QPushButton* m_confirmButton;
		QListWidget* m_listWidget;

		void OnTextChanged(const QString& text);

	public slots:
		void AddItems(const QStringList& list);

	private slots:
		void OnConfirmClicked();
	};
}