#ifndef MULTISELECTCOMBOBOX_H
#define MULTISELECTCOMBOBOX_H

#include "CustomListView.h"
#include "global.h"

#include <QComboBox>
#include <QStandardItemModel>
#include <QApplication>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QListView>

class MultiSelectComboBox : public QComboBox {
    Q_OBJECT

signals:
    void customActivated();

private:
    CustomListView *listView;

    // 当前选择的选项列表
    QStringList selectedItemList;

    // 为了解决一个奇怪的bug(当选择了两个选项后, 取消其中一个, 另一个在失去焦点时会自动取消)
    // 实际点击的选项
    QStandardItem* realClickItem = nullptr;

    // 是否为单选模式
    bool isSingleSelectionMode = false;

public:
    MultiSelectComboBox(QStringList selectedItemList = {}, QWidget *parent = nullptr) : QComboBox(parent) {
        this->selectedItemList = selectedItemList;

        //setItemDelegate(new CheckBoxDelegate(this));
        //setModel(new QStandardItemModel(this));

        QStandardItemModel *model = new QStandardItemModel(this);
        listView = new CustomListView(this);
        listView->setModel(model);
        listView->setSelectionMode(QAbstractItemView::MultiSelection);
        listView->setItemDelegate(new CheckBoxDelegate(listView));
        setView(listView);

        // 设置下拉框为可多选模式
        setEditable(true);
        // lineEdit只读
        lineEdit()->setReadOnly(true);
        // 点击lineEdit时显示下拉框
        lineEdit()->installEventFilter(this);

        // 连接信号，处理选择变化
        connect(this, QOverload<int>::of(&QComboBox::activated),this, &MultiSelectComboBox::itemClicked);
        connect(listView, &CustomListView::itemClicked, [=](QStandardItem* clickItem){
            realClickItem = clickItem;
        });
    }

    // 添加选项
    void addItem(const QString &text, const QVariant &userData = QVariant()) {
        QStandardItem *item = new QStandardItem(text);
        //item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        item->setData(userData);

        ((QStandardItemModel *)model())->appendRow(item);
    }

    // 获取选中的项目
    QStringList selectedItems() const {
        QStringList selected;
        QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model());
        if (m) {
            for (int i = 0; i < m->rowCount(); ++i) {
                QStandardItem *item = m->item(i);
                if (item->checkState() == Qt::Checked) {
                    selected << item->text();
                }
            }
        }
        return selected;
    }

    // 将选项设置为已选状态
    void setItemSelected(QString selectedItemsStr){
        auto selectedItemList = selectedItemsStr.split(KEYBOARD_COMBINE_KEY_SPE);
        // 遍历model
        QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model());
        if (m) {
            for (int i = 0; i < m->rowCount(); ++i) {
                QStandardItem *item = m->item(i);
                if (selectedItemList.contains(item->text())) {
                    item->setCheckState(Qt::Checked);
                }
            }
        }

        // 更新显示已选项
        updateText();
    }

    // 设置为单选/多选模式
    void setSelectionMode(bool isSingleSelection){
        this->isSingleSelectionMode = isSingleSelection;

        updateText();
    }

private slots:
    void itemClicked(int index) {
        QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model());
        if (m) {
            QStandardItem *currentItem = m->item(index);
            if (realClickItem && currentItem && currentItem == realClickItem) {
                // 单选模式, 取消掉其它的已选择项
                if(isSingleSelectionMode){
                    for(int i = 0; i < m->rowCount(); i++){
                        m->item(i)->setCheckState(Qt::Unchecked);
                    }
                }

                currentItem->setCheckState(currentItem->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);

                updateText();

                // 发送下拉框activated信号
                emit customActivated();
            }
        }

        realClickItem = nullptr;
    }

private:
    void updateText() {
        QStringList selected = selectedItems();

        //qDebug() << "selected.size() : " << selected.size();

        QString text = selected.join(KEYBOARD_COMBINE_KEY_SPE);

        lineEdit()->setText(text);
        lineEdit()->setToolTip(text);
    }

    // 事件过滤器
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched == lineEdit() && event->type() == QEvent::MouseButtonPress) {
            showPopup(); // 显示下拉框
            return true;
        }
        return QComboBox::eventFilter(watched, event);
    }

    // 自定义下拉选项委托
    class CheckBoxDelegate : public QStyledItemDelegate {
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override {
            // 完全禁用所有默认绘制
            painter->save();

            // 绘制背景
            if (option.state & QStyle::State_Selected) {
                painter->fillRect(option.rect, option.palette.highlight());
            } else {
                painter->fillRect(option.rect, option.palette.base());
            }

            // 绘制复选框
            Qt::CheckState state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            QStyleOptionButton checkOpt;
            checkOpt.rect = QRect(option.rect.x() + 4,
                                  option.rect.y() + (option.rect.height() - 16)/2,
                                  16, 16);
            checkOpt.state = option.state;
            checkOpt.state |= (state == Qt::Checked) ? QStyle::State_On : QStyle::State_Off;
            QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkOpt, painter);

            // 绘制文本
            painter->setPen(option.state & QStyle::State_Selected ?
                                option.palette.highlightedText().color() :
                                option.palette.text().color());

            QRect textRect = option.rect.adjusted(24, 0, 0, 0);
            QString text = index.data(Qt::DisplayRole).toString();
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);

            painter->restore();
        }

        QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override {
            QSize size = QStyledItemDelegate::sizeHint(option, index);
            size.setWidth(size.width() + 24);
            return size;
        }

        bool editorEvent(QEvent *event, QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) override{
            // 确保不在这里处理选择逻辑
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }
    };
};


#endif // MULTISELECTCOMBOBOX_H
