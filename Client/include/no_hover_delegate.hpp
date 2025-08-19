#pragma once
#include <QStyledItemDelegate>

class NoHoverDelegate final : public QStyledItemDelegate
{
public:
	explicit NoHoverDelegate(QObject* parent = nullptr);
	void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};