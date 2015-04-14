#ifndef REPORTEDITWIDGET_H
#define REPORTEDITWIDGET_H

#include <QWidget>
class QMdiSubWindow;
class DiagnosticTemplateModel;
class DiagnosticTemplateView;
class ThumbnailBarWidget;
class DicomImageLabel;
class ReportWidget;

namespace Ui {
class ReportEditWidget;
}

class ReportEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportEditWidget(QWidget *parent = 0);
    ~ReportEditWidget();

    void openReports(const QStringList &files);
    void openReport(const QString &file);
    void createReport(const QString &studyUid);
    bool setActiveReport(const QString &studyUid);

signals:
    void reportCreated();

public slots:
    void closeAllReports();

protected slots:
    void onOpenReport();
    void onSaveReport();
    void onCompleteReport();
    void onVerifyReport();
    void onInsertImage();
    void onRemoveImage();
    void onPrint();
    void onPrintPreview();
    void onSettings();
    void onPaperChanged(const QString &paper);
    void onTemplateItemSelected(const QString &findings, const QString &conclusion);
    void onSubWindowActivated(QMdiSubWindow *win);

    void onImageDoubleClicked(DicomImageLabel *label);

protected:
    void closeEvent(QCloseEvent *e);

private:
    void init();
    void setupConnections();
    void setPermissions();
    void setReadOnly(bool yes);
    void insertReportImages(ReportWidget *report);
    QMdiSubWindow *searchReport(const QString &file);
    Ui::ReportEditWidget *ui;

    QMdiSubWindow *currentWindow;
    DiagnosticTemplateModel *templateModel;
    DiagnosticTemplateView *templateView;
    ThumbnailBarWidget *thumbnailWidget;
};

#endif // REPORTEDITWIDGET_H
