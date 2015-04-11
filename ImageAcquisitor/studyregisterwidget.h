#ifndef STUDYREGISTERWIDGET_H
#define STUDYREGISTERWIDGET_H

#include <QWidget>
class StudyRecord;
class WorklistItemModel;
class QSortFilterProxyModel;
class WlistSCUThread;
class DicomScp;

namespace Ui {
class StudyRegisterWidget;
}

class StudyRegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StudyRegisterWidget(QWidget *parent = 0);
    ~StudyRegisterWidget();

signals:
    void startAcq(const StudyRecord &study);

public slots:
    void onWlistScpUpdated(const QList<DicomScp*> &scps);

protected slots:
    void onWlistToday();
    void onWlistThisWeek();
    void onWlistThisMonth();
    void onWlistSearch(bool checked);
    void onWlistScuFinished();
    void onWlistScpEcho();
    void onWlistClear();
    void onWlistDoubleClicked(const QModelIndex &index);
    void onWlistBeginStudy();
    void onWlistNewStudy();
    void onWlistEmergency();

private:
    void init();
    void createConnections();
    void clearWlistScps();
    void setPermissions();
    Ui::StudyRegisterWidget *ui;

    WorklistItemModel *wlistModel;
    QSortFilterProxyModel *wlistProxyModel;
    WlistSCUThread *wlistThread;
};

#endif // STUDYREGISTERWIDGET_H
