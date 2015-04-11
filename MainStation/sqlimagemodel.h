#ifndef SQLIMAGEMODEL_H
#define SQLIMAGEMODEL_H

#include <QSqlTableModel>

class SqlImageModel : public QSqlTableModel
{
    Q_OBJECT
public:
    enum ColumnType {
        ImageUid,
        SopClassUid,
        SeriesUid,
        StudyUid,
        RefImageUid,
        ImageNo,
        ImageTime,
        BodyPart,
        ImageDesc,
        ImageFile,

        ColumnCount,
    };

    explicit SqlImageModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

    QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal, int role = Qt::DisplayRole) const;

    QStringList getAllImageFiles() const;

signals:
    void viewImages(const QStringList &imageFiles);
    void exportImages(const QStringList &imageFiles);
    void sendImages(const QStringList &imageFiles);

public slots:
    void onViewImage(const QModelIndex &index);
    void onViewAllImages() { emit viewImages(getAllImageFiles()); }
    void onExportImage(const QModelIndex &index);
    void onExportAllImages() { emit exportImages(getAllImageFiles()); }
    void onSendImage(const QModelIndex &index);
    void onSendAllImages() { emit sendImages(getAllImageFiles()); }
    void onRemoveImage(const QModelIndex &index);
    void onRemoveAllImages();

    void onStudySelected(const QStringList &studyUids);

};

#endif // SQLIMAGEMODEL_H
