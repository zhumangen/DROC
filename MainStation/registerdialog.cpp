#include "registerdialog.h"
#include "ui_registerdialog.h"

#include <QtNetwork/QNetworkInterface>
#include <QSettings>
#include <QMessageBox>

#define PASSPORT "USERPASSPORT"
#define DEFAULT_MAC "78843C343FCB"

static uint encrypt(uint id);
static uint mac2id(QByteArray &mac);

RegisterDialog::RegisterDialog(QWidget *parent) :
    id(0),
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOk()));
    init();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::init()
{
    QList<QNetworkInterface> netlist = QNetworkInterface::allInterfaces();

    QByteArray mac;
    if (netlist.size() > 0) {
        mac = netlist.at(0).hardwareAddress().remove(':').toLatin1();
    } else {
        mac = QByteArray(DEFAULT_MAC);
    }

    id = mac2id(mac);

    ui->deviceIdEdit->setText(QString("%1").arg(id, 8, 16, QChar('0')).toUpper());
}

void RegisterDialog::onOk()
{
    bool ok;
    uint passport = ui->registerCodeEdit->text().toUInt(&ok, 16);
    if (!ok) {
        QMessageBox::critical(this, tr("User Register"), tr("Register number format error."));
        return;
    }

    if (encrypt(id) == passport){
        QSettings().setValue(PASSPORT, passport);
        accept();
    } else
        QMessageBox::critical(this, tr("User Register"), tr("Invalid register number."));
}

bool RegisterDialog::registerDevice()
{
    RegisterDialog dialog;
    if (dialog.exec() == QDialog::Accepted) return true;
    else return false;
}

bool RegisterDialog::verifyDevice()
{
    bool verified = false;
    QSettings settings;
    QList<QNetworkInterface> netlist = QNetworkInterface::allInterfaces();
    uint passport = settings.value(PASSPORT).toUInt();

    if (netlist.size() == 0) {
        if (encrypt(mac2id(QByteArray(DEFAULT_MAC))) == passport) {
            verified = true;
        }
    } else {
        foreach (QNetworkInterface net, netlist) {
            QByteArray mac = net.hardwareAddress().remove(':').toLatin1();
            if (encrypt(mac2id(mac)) == passport) {
                verified = true;
                break;
            }
        }
    }

    return verified;
}

uint mac2id(QByteArray &mac)
{
    int id = 0;
    if (mac.size() < 8) mac.append(DEFAULT_MAC);
    id += mac[0] % 16;
    for (int i = 1; i < 8; ++i) {
        id *= 16;
        id += mac[i] % 16;
    }
    return id;
}

uint encrypt(uint id)
{
    QByteArray idstr = QString("%1").arg(id, 8, 16, QChar('0')).toUpper().toLatin1();
    QByteArray macdef = QByteArray(DEFAULT_MAC);
    uint encrypted = (idstr[0]+macdef[0]) % 16;
    for (int i = 1; i < 8; ++i) {
        encrypted *= 16;
        encrypted += (idstr[i]+macdef[i]) % 16;
    }
    return encrypted;
}
