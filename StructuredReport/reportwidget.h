#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>
class DVInterface;
class QLabel;
class QPrinter;
class DicomImageView;
class ImageInstance;

namespace Ui {
class ReportWidget;
}

class ReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportWidget(QWidget *parent = 0);
    ~ReportWidget();

    bool openReport(const QString &file, bool external = false);
    bool createReport(const QString &studyUid);
    bool isReportModified() const { return isModified; }
    bool isReportNormal() const { return isNormal; }
    bool isReportVerified() const;
    QString getReportTitle() const;
    void setReportPaper(const QString &paper);
    QString getReportPaper() const;
    QString getStudyUid() const;
    QString getReportFile() const { return reportFile; }

signals:
    void reportCreated();
    void reportClosed(const QString &reportUid);

public slots:
    void reflushReportFormat();
    void insertImage(ImageInstance *image);
    void loadEvidenceImage(ImageInstance *image);
    void removeImage();
    void saveReport();
    void completeReport();
    void verifyReport();
    void print();
    void printPreview();
    void appendTemplateContents(const QString &findings, const QString &conclusion);

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void onContentChanged() { isModified = true;}
    void doPrint(QPrinter *printer);

    void onViewChanged(DicomImageView *view);

private:
    inline void init();
    void resizeReport(const QString &paper = QString("A4"));
    void initContents();
    void initLogicalDpi();
    void setReadOnly(bool yes);
    DicomImageView *createImageView(ImageInstance *image);
    void doInsertImage(ImageInstance *image);

    Ui::ReportWidget *ui;
    DVInterface *dvi;
    QList<DicomImageView*> viewList;
    DicomImageView *currentView;

    bool isModified;
    bool isNormal;
    bool isExternal;
    bool isCreated;
    QString reportFile;

    int xLogicalDpi;
    int yLogicalDpi;

    QAction *zoomAction;
    QAction *panAction;
    QAction *roiAction;
};

#endif // REPORTWIDGET_H
