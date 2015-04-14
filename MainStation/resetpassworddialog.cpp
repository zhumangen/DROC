#include "resetpassworddialog.h"
#include "ui_resetpassworddialog.h"
#include "usergroupmodels.h"
#include <QMessageBox>
#include <QModelIndex>

ResetPasswordDialog::ResetPasswordDialog(const QModelIndex &index, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResetPasswordDialog)
{
    ui->setupUi(this);
    user = static_cast<User*>(index.internalPointer());
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOk()));
}

ResetPasswordDialog::~ResetPasswordDialog()
{
    delete ui;
}

void ResetPasswordDialog::onOk()
{
    if (ui->newPW2Edit->text() != ui->newPWEdit->text()) {
        QMessageBox::critical(this, tr("Reset User Password"),
                              tr("Inconsistent new passwords."));
        return;
    }

    QString oldpw = ui->oldPWEdit->text();

    if (encryptPassword(oldpw) == user->password) {
        user->password = encryptPassword(ui->newPWEdit->text().toLatin1());
        accept();
    } else {
        QMessageBox::critical(this, tr("Reset User Password"),
                              tr("Incorrect old password, please try again."));
        return;
    }
}
