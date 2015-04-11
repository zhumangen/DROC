#ifndef PRINTSCU_H
#define PRINTSCU_H

#ifndef PRINTSCU_CPP
#define PRINTSCU_EXTERN extern
#else
#define PRINTSCU_EXTERN
#endif

class DicomImageView;
class DicomScp;
#include <QList>
#include "dcmtk/ofstd/ofcond.h"

PRINTSCU_EXTERN OFCondition printscu(const QList<DicomImageView*> &viewList,
                                     const DicomScp &printer,
                                     int copies);

#endif // PRINTSCU_H
