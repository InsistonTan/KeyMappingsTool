#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include<QListView>
#include<QStandardItemModel>
#include<QMouseEvent>

class CustomListView : public QListView {
    Q_OBJECT
public:
    using QListView::QListView;

signals:
    void itemClicked(QStandardItem* clickItem);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        QModelIndex index = indexAt(event->pos());
        if (!index.isValid()) {
            QListView::mousePressEvent(event);
            return;
        }

        // 获取模型项
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
        if (!model) {
            QListView::mousePressEvent(event);
            return;
        }

        QStandardItem* item = model->itemFromIndex(index);

        emit itemClicked(item);
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        e->accept(); // 阻止默认行为
    }

    void mouseDoubleClickEvent(QMouseEvent *e) override {
        e->accept(); // 阻止默认行为
    }
};

#endif // CUSTOMLISTVIEW_H
