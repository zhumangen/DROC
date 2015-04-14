#ifndef FINDSCUTHREAD_H
#define FINDSCUTHREAD_H

#include <QThread>
#include <QDate>
#include "../share/dicomscp.h"
#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dimse.h"
class FindSCUCallback;
class DcmDataset;
class DcmFileFormat;

class FindSCUThread : public QThread
{
    Q_OBJECT
public:
    typedef enum {
         QMPatientRoot = 0,
         QMStudyRoot = 1,
         QMPatientStudyOnly = 2
    } QueryModel;

    typedef struct {
        const char *findSyntax;
        const char *moveSyntax;
    } QuerySyntax;

    typedef struct {
        T_ASC_Association *assoc;
        T_ASC_PresentationContextID presId;
    } MyCallbackInfo;

    struct StoreCallbackData
    {
      char* imageFileName;
      DcmFileFormat* dcmff;
      T_ASC_Association* assoc;
    };

    explicit FindSCUThread(FindSCUCallback *callback, QObject *parent = 0);

    void run();
    void setQRScp(const DicomScp &scp) { qrscp = scp; }
    void setPatientId(const QString &id) { patientId = id; }
    void setPatientName(const QString &name) { patientName = name; }
    void setStudyDescription(const QString &desc) { studyDesc = desc; }
    void setModality(const QString &modality) { this->modality = modality; }
    void setFromDate(const QDate &date) { fromDate = date; }
    void setToDate(const QDate &date) { toDate = date; }
    void setAbort(bool yes = true) { abort = yes; }

signals:
    void progressMsg(const QString &msg);

public slots:

private:
    OFCondition moveSCU(T_ASC_Association * assoc, const char *fname);
    void substituteOverrideKeys(DcmDataset *dset);
    static void moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *request,
        int responseCount, T_DIMSE_C_MoveRSP *response);
    static void subOpCallback(void * /*subOpCallbackData*/ ,
            T_ASC_Network *aNet, T_ASC_Association **subAssoc);
    static OFCondition subOpSCP(T_ASC_Association **subAssoc);
    static OFCondition storeSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg,
            T_ASC_PresentationContextID presID);
    static void storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress,
        T_DIMSE_C_StoreRQ *req, char *imageFileName, DcmDataset **imageDataSet,
        T_DIMSE_C_StoreRSP *rsp, DcmDataset **statusDetail);
    static OFCondition acceptSubAssoc(T_ASC_Network * aNet, T_ASC_Association ** assoc);
    OFCondition addPresentationContext(T_ASC_Parameters *params, T_ASC_PresentationContextID pid,
                            const char* abstractSyntax);

    FindSCUCallback *callback_;
    static bool abort;
    DicomScp qrscp;
    QString patientId;
    QString patientName;
    QString studyDesc;
    QString modality;
    QDate fromDate;
    QDate toDate;

    T_ASC_Network *net;
    DcmDataset *overrideKeys;
    static QuerySyntax querySyntax[3];

};

#endif // FINDSCUTHREAD_H
