#pragma once

#include "CustomListView.h"
#include "common/Global.h"
#include "qnamespace.h"

#include <QComboBox>
#include <QStandardItemModel>
#include <QApplication>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QListView>
#include <QTimer>

class MultiSelectComboBox : public QComboBox {
    Q_OBJECT

signals:
    void selectionChanged();

private:
    CustomListView *listView;

    // 当前选择的选项列表
    QStringList m_selectedItemList;

    // 为了解决一个奇怪的bug(当选择了两个选项后, 取消其中一个, 另一个在失去焦点时会自动取消)
    // 实际点击的选项
    QStandardItem* realClickItem = nullptr;

    // 是否为单选模式
    bool isSingleSelectionMode = false;

    // 是否折叠文本
    bool m_isElidedText;

    // placeholder
    QString m_placeholder;

public:
    MultiSelectComboBox(bool isElidedText = true, QWidget *parent = nullptr) : QComboBox(parent) {
        this->m_isElidedText = isElidedText;

        QStandardItemModel *model = new QStandardItemModel(this);
        listView = new CustomListView(this);
        listView->setModel(model);
        listView->setSelectionMode(QAbstractItemView::MultiSelection);
        listView->setItemDelegate(new CheckBoxDelegate(this, listView));
        setView(listView);

        // 禁用lineEdit
        setEditable(false);

        // 设置事件过滤器, 鼠标点击显示下拉框
        this->installEventFilter(this);

        // 连接信号，处理选择变化
        connect(this, QOverload<int>::of(&QComboBox::activated),this, &MultiSelectComboBox::itemClicked);
        connect(listView, &CustomListView::itemClicked, [=](QStandardItem* clickItem){
            realClickItem = clickItem;
        });
    }

    // 添加选项
    void addItem(const QString &text, const QVariant &userData = QVariant()) {
        QStandardItem *item = new QStandardItem(text);

        item->setCheckState(Qt::Unchecked);
        item->setData(userData);
        item->setToolTip(text);

        ((QStandardItemModel *)model())->appendRow(item);
    }

    // 获取选中的项目
    QStringList selectedItems() const {
        return m_selectedItemList;
    }

    // 将选项设置为已选状态
    void setItemSelected(QString selectedItem){
        // 遍历model
        QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model());
        if (m) {
            for (int i = 0; i < m->rowCount(); ++i) {
                QStandardItem *item = m->item(i);
                if (selectedItem == item->text()) {
                    item->setCheckState(Qt::Checked);
                    if(!m_selectedItemList.contains(item->text())){
                        m_selectedItemList.append(selectedItem);
                    }
                    break;
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

    // placeholderText
    void setPlaceholderText(const QString &text){
        m_placeholder = text;
        QTimer::singleShot(0, this, [this] {
            updateText();   // 延迟刷新
        });
    }
    QString placeholderText() const
    {
        return m_placeholder;
    }

    // 折叠文本
    bool isElidedText(){return m_isElidedText;}
    void setElidedText(bool val){
        m_isElidedText = val;
    }

    // 清空已选
    void clear(){
        // 清空已选项
        m_selectedItemList.clear();

        // 清空下拉列表项
        if (auto *m = qobject_cast<QStandardItemModel*>(model())) {
            m->removeRows(0, m->rowCount());
        }
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
                if(currentItem->checkState()){
                    if(!m_selectedItemList.contains(currentItem->text())){
                        m_selectedItemList.append(currentItem->text());
                    }
                }else {
                    m_selectedItemList.removeAll(currentItem->text());
                }

                updateText();

                // 发送下拉框activated信号
                emit selectionChanged();
            }
        }

        realClickItem = nullptr;
    }

private:
    // 绘制下拉框显示的文本
    void paintEvent(QPaintEvent *e) override
    {
        Q_UNUSED(e);

        QStyleOptionComboBox opt;
        initStyleOption(&opt);

        QPainter p(this);

        // =========================
        // 1. 画 ComboBox 外框 + arrow
        // =========================
        style()->drawComplexControl(
            QStyle::CC_ComboBox,
            &opt,
            &p,
            this
            );

        // =========================
        // 2. 获取 text 区域（关键）
        // =========================
        QRect textRect = style()->subControlRect(
            QStyle::CC_ComboBox,
            &opt,
            QStyle::SC_ComboBoxEditField,
            this
            );

        textRect.adjust(6, 0, -6, 0); // 🔥防止贴边吃字

        // =========================
        // 3. 获取显示文本
        // =========================
        QStringList selected = selectedItems();

        QString text = selected.isEmpty()
                           ? m_placeholder
                           : selected.join(KEYBOARD_COMBINE_KEY_SPE);

        // =========================
        // 4. elide
        // =========================
        QString elided = fontMetrics().elidedText(
            text,
            Qt::ElideMiddle,
            textRect.width()
            );

        // =========================
        // 5. 颜色（区分 placeholder）
        // =========================
        QColor color;
        if (selected.isEmpty()) {
            color = palette().color(QPalette::PlaceholderText);
        } else {
            color = palette().color(QPalette::Text);
        }

        p.setPen(color);

        // =========================
        // 6. 绘制文本
        // =========================
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, isElidedText() ? elided : text);

        setToolTip(text);
    }

    void updateText() {
        update();
    }

    // 事件过滤器
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched == this && event->type() == QEvent::MouseButtonPress) {
            showPopup(); // 显示下拉框
            return true;
        }
        return QComboBox::eventFilter(watched, event);
    }

    // 自定义下拉选项委托
    class CheckBoxDelegate : public QStyledItemDelegate {
    private:
        MultiSelectComboBox* srcComboBox;
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        explicit CheckBoxDelegate(MultiSelectComboBox* combo, QObject* parent = nullptr)
            : QStyledItemDelegate(parent), srcComboBox(combo)
        {}

        void paint(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override {
            // 完全禁用所有默认绘制
            painter->save();

            // 获取复选框状态
            Qt::CheckState checkState = static_cast<Qt::CheckState>(
                index.data(Qt::CheckStateRole).toInt()
                );

            // 绘制背景
            QColor bgColor;
            QColor textColor;
            // 优先判断选中（选中的优先级高于悬停）
            if (checkState == Qt::Checked) {
                bgColor = QColor("#2563EB");
                textColor = QColor("#ffffff");
            }
            // 其次判断悬停（对应你的样式表 :hover）
            else if (option.state & QStyle::State_MouseOver) {
                bgColor = QColor("#1F2937");  // 悬停背景色
                textColor = QColor("#F9FAFB"); // 悬停文字色
            }
            // 默认状态
            else {
                bgColor = option.palette.base().color();
                textColor = option.palette.text().color();
            }
            painter->fillRect(option.rect, bgColor);


            const int indicatorSize = 16;
            const int leftMargin = 4;
            const int spacing = 8;

            // // 绘制复选框
            // Qt::CheckState state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            // QStyleOptionButton checkOpt;
            // checkOpt.rect = QRect(
            //     option.rect.left() + leftMargin,
            //     option.rect.center().y() - indicatorSize / 2,
            //     indicatorSize,
            //     indicatorSize
            //     );
            // checkOpt.state = option.state;
            // checkOpt.state |= (state == Qt::Checked) ? QStyle::State_On : QStyle::State_Off;
            // QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkOpt, painter);

            QRect r(
                option.rect.left() + leftMargin,
                option.rect.center().y() - indicatorSize / 2,
                indicatorSize,
                indicatorSize
                );

            QColor borderColor("#6B7280");
            QColor fillColor("#2563EB");

            painter->save();

            painter->setRenderHint(QPainter::Antialiasing);

            QPen pen(borderColor);
            pen.setWidth(1);

            painter->setPen(pen);

            if(checkState == Qt::Checked)
            {
                pen.setColor(QColor(Theme::textColor()));
                painter->setPen(pen);
                painter->setBrush(fillColor);
            }
            else
            {
                painter->setBrush(Qt::NoBrush);
            }

            painter->drawRoundedRect(r,4,4);

            if(checkState == Qt::Checked)
            {
                QPen checkPen(Qt::white);
                checkPen.setWidth(2);
                checkPen.setCapStyle(Qt::RoundCap);
                checkPen.setJoinStyle(Qt::RoundJoin);

                painter->setPen(checkPen);
                painter->drawLine(
                    r.left()+4,
                    r.center().y(),
                    r.center().x()-1,
                    r.bottom()-4
                    );
                painter->drawLine(
                    r.center().x()-1,
                    r.bottom()-4,
                    r.right()-4,
                    r.top()+4
                    );
            }
            painter->restore();


            // 文本
            QRect textRect = option.rect.adjusted(
                    leftMargin + indicatorSize + spacing,
                    0,
                    -4,
                    0
                    );

            QString text = index.data(Qt::DisplayRole).toString();

            QString elided = option.fontMetrics.elidedText(
                text,
                Qt::ElideMiddle,
                textRect.width()
                );

            painter->setPen((option.state & QStyle::State_Selected)
                                ? option.palette.highlightedText().color()
                                : option.palette.text().color());

            painter->drawText(textRect,
                              Qt::AlignVCenter | Qt::AlignLeft,
                              srcComboBox->isElidedText() ? elided : text);


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


