#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "../share/global.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

    static bool userLogin(User &user, Group &group);

protected slots:
    void verifyPassword();

private:
    Ui::LoginDialog *ui;
    User _user;
    Group _group;
};

#endif // LOGINDIALOG_H
