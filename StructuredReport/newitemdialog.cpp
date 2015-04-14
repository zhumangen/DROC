#include "newitemdialog.h"
#include "ui_newitemdialog.h"

#include <QMessageBox>

NewItemDialog::NewItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewItemDialog)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOk()));
}

void NewItemDialog::setContents(const QString &t, const QString &f, const QString &c)
{
    ui->titleEdit->setText(t);
    ui->findingsEdit->setPlainText(f);
    ui->conclusionEdit->setPlainText(c);
}

NewItemDialog::~NewItemDialog()
{
    delete ui;
}

void NewItemDialog::onOk()
{
    title = ui->titleEdit->text();
    findings = ui->findingsEdit->toPlainText();
    conclusion = ui->conclusionEdit->toPlainText();

    if (title.isEmpty()) {
        QMessageBox::critical(this, tr("Diagnostic Template"),
                              tr("Title is empty."));
    } else accept();
}
