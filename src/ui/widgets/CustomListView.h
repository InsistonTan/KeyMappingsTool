#pragma once

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

        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
        if (model) {
            QStandardItem* item = model->itemFromIndex(index);
            emit itemClicked(item);
        }

        // 必须调用 base class
        QListView::mousePressEvent(event);
    }

    // void mouseReleaseEvent(QMouseEvent *e) override {
    //     e->accept(); // 阻止默认行为
    // }

    // void mouseDoubleClickEvent(QMouseEvent *e) override {
    //     e->accept(); // 阻止默认行为
    // }
};

