#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = 0);
    ~RegisterDialog();

    static bool verifyDevice();
    static bool registerDevice();

protected slots:
    void onOk();

private:
    void init();
    uint id;
    Ui::RegisterDialog *ui;
};

#endif // REGISTERDIALOG_H
