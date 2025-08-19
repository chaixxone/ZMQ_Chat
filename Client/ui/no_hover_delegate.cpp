#include <no_hover_delegate.hpp>

NoHoverDelegate::NoHoverDelegate(QObject* parent) : QStyledItemDelegate(parent)
{

}

void NoHoverDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
	QStyledItemDelegate::initStyleOption(option, index);
	option->state &= ~QStyle::State_MouseOver;
}