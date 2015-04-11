#ifndef SENDSTUDYDIALOG_H
#define SENDSTUDYDIALOG_H

#include <QDialog>
class StoreSCUThread;
class DicomScp;

namespace Ui {
class SendStudyDialog;
}

class SendStudyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendStudyDialog(QWidget *parent = 0);
    ~SendStudyDialog();

    void setImageFiles(const QStringList &imageFiles);
    void setCurrentScp(const QString &scpid);
    void startSend() { onSend(true); }
    void setCloseOnFinish(bool yes) { closeOnFinish = yes; }

protected:
    void closeEvent(QCloseEvent *e);

protected slots:
    void onSend(bool checked);
    void onScuThreadFinished();
    void onResultReady(const QString &msg);

private:
    void init();

    QList<DicomScp*> scpList;
    Ui::SendStudyDialog *ui;
    StoreSCUThread *scuThread;
    bool closeOnFinish;
    int imageCount;
};

#endif // SENDSTUDYDIALOG_H
