#ifndef FINDSCUTHREAD_H
#define FINDSCUTHREAD_H

#include <QThread>
#include <QDateTime>
#include "../share/dicomscp.h"
#include "dcmtk/dcmnet/dfindscu.h"
class WorklistItemModel;


class WlistSCUCallback: public DcmFindSCUCallback
{
public:
    WlistSCUCallback(WorklistItemModel *wlistModel) :
        abort(0),
        model(wlistModel),
        DcmFindSCUCallback() {}

    void callback(T_DIMSE_C_FindRQ *request,
                  int responseCount,
                  T_DIMSE_C_FindRSP *rsp,
                  DcmDataset *responseIdentifiers);

    void setAbort(bool yes) { abort = yes; }

private:
    bool abort;
    WorklistItemModel *model;
};

class WlistSCUThread : public QThread
{
    Q_OBJECT
public:
    explicit WlistSCUThread(WorklistItemModel *model, QObject *parent = 0) :
        callback_(new WlistSCUCallback(model)),
        QThread(parent) {}
    ~WlistSCUThread() { delete callback_; }

    void run();
    void setWorklistScp(const DicomScp &scp) { wlistScp = scp; }
    void setFindAETitle(const QString &aet) { findAE = aet; }
    void setAccNumber(const QString &accNum) { accNumber = accNum; }
    void setPatientId(const QString &id) { patientId = id; }
    void setPatientName(const QString &name) { patientName = name; }
    void setProcedureId(const QString &id) { procId = id; }
    void setModality(const QString &modality) { this->modality = modality; }
    void setFromTime(const QDateTime &time) { fromTime = time; }
    void setToTime(const QDateTime &time) { toTime = time; }
    void setAbort(bool yes) { callback_->setAbort(yes); }

signals:
    void progressMsg(const QString &msg);

public slots:

private:
    WlistSCUCallback *callback_;
    DicomScp wlistScp;
    QString findAE;
    QString accNumber;
    QString patientId;
    QString patientName;
    QString procId;
    QString modality;
    QDateTime fromTime;
    QDateTime toTime;
};


#endif // FINDSCUTHREAD_H
