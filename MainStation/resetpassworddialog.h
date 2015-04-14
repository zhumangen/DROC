#ifndef RESETPASSWORDDIALOG_H
#define RESETPASSWORDDIALOG_H

#include <QDialog>
#include "usergroupmodels.h"

namespace Ui {
class ResetPasswordDialog;
}

class ResetPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetPasswordDialog(const QModelIndex &index, QWidget *parent = 0);
    ~ResetPasswordDialog();

protected slots:
    void onOk();

private:
    User *user;
    Ui::ResetPasswordDialog *ui;
};

#endif // RESETPASSWORDDIALOG_H
