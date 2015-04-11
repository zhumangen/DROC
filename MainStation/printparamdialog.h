#ifndef PRINTPARAMDIALOG_H
#define PRINTPARAMDIALOG_H

#include <QDialog>
struct PrintParam;

namespace Ui {
class PrintParamDialog;
}

class PrintParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrintParamDialog(PrintParam &params, QWidget *parent = 0);
    ~PrintParamDialog();

protected slots:
    void onOk();

private:
    void init();
    void loadPrintConfig();
    Ui::PrintParamDialog *ui;
    PrintParam &param;
};

#endif // PRINTPARAMDIALOG_H
