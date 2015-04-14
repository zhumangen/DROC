#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include <QWidget>
class ScanDcmFileThread;
class StudyRecord;
class ImportStudyModel;
class ImportStudyView;
class QSqlTableModel;
class ImportDcmFileThread;

namespace Ui {
class ArchiveWidget;
}

class ArchiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArchiveWidget(QWidget *parent = 0);
    ~ArchiveWidget();

signals:
    void importFinished();

protected slots:
    void onFileClicked();
    void onFolderClicked();
    void onScanImport(bool checked);
    void onScanThreadFinished();
    void onImportThreadFinished();
    void onResultReady();

    void onLogSearch();
    void onLogExportClicked();

protected:
    void closeEvent(QCloseEvent *e);

private:
    void init();
    void createConnections();
    void setImportButtonsDisabled(bool yes);
    void scanFiles(const QStringList &files);
    void setPermissions();

    Ui::ArchiveWidget *ui;
    ScanDcmFileThread *scanThread;
    ImportDcmFileThread *importThread;
    ImportStudyModel *importModel;
    ImportStudyView *importView;
    QSqlTableModel *logModel;
};

#endif // ARCHIVEWIDGET_H
