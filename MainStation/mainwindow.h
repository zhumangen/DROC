#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../share/global.h"

class StudyRecord;
class ReportEditWidget;
class ImageViewWidget;
class StudyRegisterWidget;
class ImageAcquisitWidget;
class StudyExplorerWidget;
class ArchiveWidget;
class SystemSettingsWidget;
class ExitWidget;
class ProcedureItemModel;
class PerspectiveViewWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    explicit MainWindow(const User &u, const Group &g, QWidget *parent = 0);
    ~MainWindow();

    const User& getCurrentUser() const { return currentUser; }
    const Group& getCurrentGroup() const { return currentGroup; }
    const InstitutionInfo& getInstInfo() const;
    const ManufactureInfo& getManuInfo() const;
    const StationInfo& getStationInfo() const;
    const CommunicationInfo& getCommInfo() const;
    const WorkFlow& getWorkFlow() const;
    QString getDbLocation() const;
    const CustomizedId& getPatientIdFormat() const;
    const CustomizedId& getAccNumFormat() const;
    ProcedureItemModel *getProcModel() const;


protected slots:
    void onViewImages(const QStringList &imageFiles);
    void onCreateReport(const QString &studyUid);
    void onviewReports(const QStringList &reportFiles);
    void onStartAcq(const StudyRecord &study);
    void onEndAcq();

    void onCurrentTabChanged();

protected:
    void closeEvent(QCloseEvent *e);

private:
    void init();
    void createComponents();
    void createConnections();
    void setupComponents();

    User currentUser;
    Group currentGroup;

    Ui::MainWindow *ui;

    ReportEditWidget *reportEditTab;
    ImageViewWidget *imageViewTab;
    StudyRegisterWidget *registerTab;
    ImageAcquisitWidget *acquisitTab;
    StudyExplorerWidget *studyExplorerTab;
    ArchiveWidget *archiveTab;
    SystemSettingsWidget *settingsTab;
    PerspectiveViewWidget *spectViewTab;
    ExitWidget *exitTab;
    QWidget *prevTab;
};

#ifdef MAINWINDOW_CPP
#define MAINWINDOW_EXTERN
#else
#define MAINWINDOW_EXTERN extern
#endif

MAINWINDOW_EXTERN const MainWindow *mainWindow;

#endif // MAINWINDOW_H
