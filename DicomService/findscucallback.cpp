#include "findscucallback.h"
#include "../share/configfiles.h"

#include "dcmtk/dcmdata/dcfilefo.h"

FindSCUCallback::FindSCUCallback():
    abort(0),
    dvi(QR_DATABASE_CFG),
    DcmFindSCUCallback()
{
}

void FindSCUCallback::callback(T_DIMSE_C_FindRQ *request, int responseCount,
                               T_DIMSE_C_FindRSP *rsp, DcmDataset *responseIdentifiers)
{
    DcmFileFormat dcmfile(responseIdentifiers);
    dvi.saveFileFormatToDB(dcmfile);

    if (abort) {
        DIMSE_sendCancelRequest(assoc_, presId_, request->MessageID);
    }
}
