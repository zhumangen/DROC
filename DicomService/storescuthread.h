#ifndef STORESCUTHREAD_H
#define STORESCUTHREAD_H

#include <QThread>
#include "../share/dicomscp.h"
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/dcmnet/assoc.h"

class StoreSCUThread : public QThread
{
    Q_OBJECT
public:
    explicit StoreSCUThread(QObject *parent = 0);

    void run();
    void setStoreScp(const DicomScp &scp) { storeScp = scp; }
    void setImageFiles(const QStringList &images) { imageFiles = images; }
    void setAbort(bool yes) { abort = yes; }


signals:
    void resultReady(const QString &msg);

public slots:

private:
    OFCondition sendImage(T_ASC_Association *assoc, const char *sopClass,
                          const char *sopInstance, const char *imgFile);
    OFCondition addAllStoragePresentationContexts(T_ASC_Parameters *params,
                                                  int opt_implicitOnly = OFFalse);

    QStringList imageFiles;
    DicomScp storeScp;
    bool abort;
};

#endif // STORESCUTHREAD_H
