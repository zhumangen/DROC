#ifndef EXPORTIMAGEDIALOG_H
#define EXPORTIMAGEDIALOG_H

#include <QDialog>
class ExportImageThread;

namespace Ui {
class ExportImageDialog;
}

class ExportImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportImageDialog(QWidget *parent = 0);
    ~ExportImageDialog();

    void setImageFiles(const QStringList &images);

protected:
    void closeEvent(QCloseEvent *e);

protected slots:
    void onStart(bool checked);
    void onSelectFolder();
    void onExportFinished();
    void onResultReady(const QString &msg);

private:
    void init();

    Ui::ExportImageDialog *ui;
    ExportImageThread *exportThread;

    int imageCount;
};

#endif // EXPORTIMAGEDIALOG_H
