#ifndef FINDSCUCALLBACK_H
#define FINDSCUCALLBACK_H

#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmpstat/dviface.h"

class FindSCUCallback: public DcmFindSCUCallback
{
public:
    FindSCUCallback();

    void callback(T_DIMSE_C_FindRQ *request,
                  int responseCount,
                  T_DIMSE_C_FindRSP *rsp,
                  DcmDataset *responseIdentifiers);

    void setAbort(bool yes) { abort = yes; }

private:
    bool abort;
    DVInterface dvi;
};

#endif // FINDSCUCALLBACK_H
