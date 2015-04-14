#include "importstudyview.h"

#include <QHeaderView>

ImportStudyView::ImportStudyView(QWidget *parent) :
    QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setMinimumSectionSize(100);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ImportStudyView::removeSelectedRows()
{
    QAbstractItemModel *model = this->model();
    if (model) {
        QList<int> rows;
        QModelIndexList indexList = selection.indexes();
        foreach (QModelIndex index, indexList) {
            if (index.column() == 0) {
                rows << index.row();
            }
        }

        qSort(rows.begin(), rows.end());
        for (int i = rows.size()-1; i >= 0; i--)
            model->removeRow(rows.at(i));
    }
}

void ImportStudyView::removeAllRows()
{
    QAbstractItemModel *model = this->model();
    if (model) model->removeRows(0, model->rowCount());
}

void ImportStudyView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    selection.merge(selected, QItemSelectionModel::Select);
    selection.merge(deselected, QItemSelectionModel::Deselect);

    QTableView::selectionChanged(selected, deselected);
}
