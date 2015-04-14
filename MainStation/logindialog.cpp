#include "logindialog.h"
#include "ui_logindialog.h"

#include "../share/configfiles.h"
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QSettings>

#define LOGIN_USER_NAMES "LOGINUSERNAMES"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(verifyPassword()));

    QSettings settings;
    ui->userIdCombo->addItems(settings.value(LOGIN_USER_NAMES).toStringList());
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::verifyPassword()
{
    QString userId = ui->userIdCombo->currentText();
    QString pwd = ui->passwordEdit->text();

    QFile file(REGISTERED_USERS);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        bool findUser = false;
        while (!in.atEnd()) {
            User user;
            in >> user;
            if (user.name == userId) {
                findUser = true;
                if (user.password == encryptPassword(pwd)) {
                    _user = user;
                    QFile file(REGISTERED_GROUPS);
                    if (file.open(QIODevice::ReadOnly)) {
                        QDataStream in(&file);
                        bool findGroup = false;
                        while (!in.atEnd()) {
                            Group group;
                            in >> group;
                            if (user.group == group.name) {
                                findGroup = true;
                                _group = group;
                                break;
                            }
                        }
                        if (!findGroup) {
                            QMessageBox::critical(this, tr("User Login"),
                                                  tr("Group of \"%1\" doesnot exist.").arg(userId));
                        } else {
                            QSettings settings;
                            QStringList names = settings.value(LOGIN_USER_NAMES).toStringList();
                            names.removeAll(userId);
                            names.prepend(userId);
                            settings.setValue(LOGIN_USER_NAMES, names);
                            accept();
                        }
                    } else {
                        QMessageBox::critical(this, tr("User Login"),
                                              tr("Open User/Group configuration file failed."));
                    }
                } else {
                    QMessageBox::critical(this, tr("User Login"), tr("Incorrect password."));
                }

                break;
            }
        }
        if (!findUser) {
            QMessageBox::critical(this, tr("User Login"), tr("User doesnot exist."));
        }
    } else {
        QMessageBox::critical(this, tr("User Login"), tr("Open User/Group configuration file failed."));
    }
}

bool LoginDialog::userLogin(User &user, Group &group)
{
    LoginDialog dialog;
    if (QDialog::Accepted == dialog.exec()) {
        user = dialog._user;
        group = dialog._group;
        return true;
    } else return false;
}
