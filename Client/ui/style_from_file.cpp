#include <style_from_file.hpp>

void UI::setStyleFromFile(QWidget* widget, const QString& path)
{
	QFile styleFile(path);

	if (!styleFile.open(QFile::ReadOnly | QFile::Text))
	{
		qWarning("Cannot open file %s for reading: %s", qPrintable(styleFile.fileName()), qPrintable(styleFile.errorString()));
		return;
	}

	QTextStream textStream(&styleFile);
	QString styleSheet = textStream.readAll();
	widget->setStyleSheet(styleSheet);
}