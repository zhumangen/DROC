#ifndef NEWITEMDIALOG_H
#define NEWITEMDIALOG_H

#include <QDialog>

namespace Ui {
class NewItemDialog;
}

class NewItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewItemDialog(QWidget *parent = 0);
    ~NewItemDialog();

    void setContents(const QString &t, const QString &f, const QString &c);

    QString title;
    QString findings;
    QString conclusion;

public slots:
    void onOk();

private:
    Ui::NewItemDialog *ui;
};

#endif // NEWITEMDIALOG_H
