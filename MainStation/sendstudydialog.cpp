#include "sendstudydialog.h"
#include "ui_sendstudydialog.h"

#include "../DicomService/storescuthread.h"
#include "../share/configfiles.h"
#include <QFile>
#include <QDataStream>
#include <QCloseEvent>

SendStudyDialog::SendStudyDialog(QWidget *parent) :
    scuThread(new StoreSCUThread(this)),
    closeOnFinish(false),
    imageCount(0),
    QDialog(parent),
    ui(new Ui::SendStudyDialog)
{
    ui->setupUi(this);
    init();
}

SendStudyDialog::~SendStudyDialog()
{
    qDeleteAll(scpList);
    delete ui;
}

void SendStudyDialog::setCurrentScp(const QString &scpid)
{
    ui->serverCombo->setCurrentText(scpid);
}

void SendStudyDialog::setImageFiles(const QStringList &imageFiles)
{
    scuThread->setImageFiles(imageFiles);
    imageCount = imageFiles.size();
    ui->textBrowser->append(tr("%1 images to send.").arg(imageCount));
}

void SendStudyDialog::init()
{
    QFile file(REMOTESCPS_CFG);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        while (!in.atEnd()) {
            DicomScp *scp = new DicomScp;
            in >> *scp;
            if (scp->type == DicomScp::ST_Storage) {
                scpList << scp;
                ui->serverCombo->addItem(scp->id, (qulonglong)scp);
            } else {
                delete scp;
            }
        }
        file.close();
    }

    if (ui->serverCombo->count()==0) ui->sendButton->setDisabled(true);
    connect(ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(onSend(bool)));
    connect(scuThread, SIGNAL(finished()), this, SLOT(onScuThreadFinished()));
    connect(scuThread, SIGNAL(resultReady(QString)), this, SLOT(onResultReady(QString)));
}

void SendStudyDialog::onSend(bool checked)
{
    if (checked) {
        DicomScp *scp = reinterpret_cast<DicomScp*>(ui->serverCombo->currentData().toULongLong());
        if (scp) {
            ui->sendButton->setText(tr("Abort"));
            ui->closeButton->setDisabled(true);
            ui->progressBar->setMaximum(imageCount);
            ui->progressBar->setValue(0);
            ui->textBrowser->append(tr("Start sending..."));
            scuThread->setAbort(false);
            scuThread->setStoreScp(*scp);
            scuThread->start();
        }
    } else {
        scuThread->setAbort(true);
    }
}

void SendStudyDialog::onScuThreadFinished()
{
    ui->sendButton->setText(tr("Send"));
    ui->sendButton->setChecked(false);
    ui->closeButton->setDisabled(false);
    if (imageCount == ui->progressBar->value()) {
        ui->textBrowser->append(tr("Sending succeeded."));
        if (closeOnFinish) accept();
    } else {
        ui->textBrowser->append(tr("Sending failed."));
    }

}

void SendStudyDialog::onResultReady(const QString &msg)
{
    ui->progressBar->setValue(ui->progressBar->value() + 1);
    ui->textBrowser->append(msg);
}

void SendStudyDialog::closeEvent(QCloseEvent *e)
{
    if (scuThread->isRunning())
        e->ignore();
}
