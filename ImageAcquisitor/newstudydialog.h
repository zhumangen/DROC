#ifndef NEWSTUDYDIALOG_H
#define NEWSTUDYDIALOG_H

#include <QDialog>
#include "../share/studyrecord.h"

namespace Ui {
class NewStudyDialog;
}

class NewStudyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewStudyDialog(QWidget *parent = 0);
    explicit NewStudyDialog(const StudyRecord &studyRec, QWidget *parent);
    ~NewStudyDialog();

    const StudyRecord& getStudyRecord() const { return study; }

public slots:
    void onOk();

private:
    void init();
    Ui::NewStudyDialog *ui;

    StudyRecord study;
    bool modifyMode;
};

#endif // NEWSTUDYDIALOG_H
