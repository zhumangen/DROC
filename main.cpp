#include "MainStation/mainwindow.h"
#include "MainStation/registerdialog.h"
#include "MainStation/logindialog.h"
#include "share/global.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setFont(QFont(QString::fromUtf8("微软雅黑"), 12));
    QCoreApplication::setApplicationName("dicomimagestation");
    QCoreApplication::setOrganizationName("Angell Medical Instrument");
    QCoreApplication::setOrganizationDomain("www.angell-med.com");

    QTranslator t;
    t.load("dicomimagestation_cn.qm");
    a.installTranslator(&t);

    // Device Register
    if (!RegisterDialog::verifyDevice()) {
        if (!RegisterDialog::registerDevice())
            return -1;
    }

    // User Login
    User user;
    Group group;
    if (!LoginDialog::userLogin(user, group))
        return -2;

    MainWindow w(user, group);
    w.showMaximized();

    return a.exec();
}
