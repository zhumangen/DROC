#-------------------------------------------------
#
# Project created by QtCreator 2014-10-09T10:56:42
#
#-------------------------------------------------

QT       += core gui printsupport network serialport sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DicomImageStation
TEMPLATE = app

DEFINES += NOMINMAX
DEFINES += _REENTRANT
DEFINES -= UNICODE
QMAKE_CFLAGS_RELEASE += /MD
QMAKE_CFLAGS_DEBUG += /MDd
QMAKE_CXXFLAGS_RELEASE += /MD
QMAKE_CXXFLAGS_DEBUG += /MDd

<<<<<<< HEAD
DCMTK = $$quote("C:\Program Files\DCMTK_MD")
=======
DCMTK = $$quote("C:\Program Files (x86)\DCMTK_MDd")
>>>>>>> ca59fabbdc88c4f2995bcfc833f54cc27c9797fa
DCMTK_INCLUDE = $$DCMTK"\include"
INCLUDEPATH += $$DCMTK_INCLUDE
DCMTK_LIB = $$DCMTK"\lib"
LIBS += $$DCMTK_LIB"\*.lib"
LIBS += advapi32.lib wsock32.lib netapi32.lib ws2_32.lib
LIBS += crapi.lib IeLauncher.lib

RC_ICONS = Resources/images/window.ico
TRANSLATIONS += dicomimagestation_cn.ts



RESOURCES += \
    resources.qrc

HEADERS += \
    DicomService/echoscu.h \
    DicomService/findscucallback.h \
    DicomService/findscuthread.h \
    DicomService/storescuthread.h \
    DicomService/wlistscuthread.h \
    GraphicsItem/abstractpathitem.h \
    GraphicsItem/graphicsangleitem.h \
    GraphicsItem/graphicsarrowitem.h \
    GraphicsItem/graphicscliprectitem.h \
    GraphicsItem/graphicscobbangleitem.h \
    GraphicsItem/graphicscrossitem.h \
    GraphicsItem/graphicsellipseitem.h \
    GraphicsItem/graphicslineitem.h \
    GraphicsItem/graphicsrectitem.h \
    GraphicsItem/graphicssimpletextitem.h \
    GraphicsItem/graphicstextmarkdialog.h \
    GraphicsItem/graphicstextmarkitem.h \
    ImageAcquisitor/bodyprotographicsview.h \
    ImageAcquisitor/imageeditgraphicsview.h \
    MainStation/aboutdialog.h \
    MainStation/exportimagedialog.h \
    MainStation/exportimagethread.h \
    MainStation/itemdelegates.h \
    MainStation/logindialog.h \
    MainStation/mainwindow.cpp.autosave \
    MainStation/mainwindow.h \
    MainStation/mainwindow.h.autosave \
    MainStation/registerdialog.h \
    MainStation/resetpassworddialog.h \
    MainStation/sendstudydialog.h \
    MainStation/usergroupmodels.h \
    share/configfiles.h \
    share/global.h \
    StructuredReport/diagnostictemplatemodel.h \
    StructuredReport/reportwidget.h \
    StructuredReport/srsettingsdialog.h \
    share/dicomscp.h \
    DicomViewer/dicomimagelabel.h \
    DicomViewer/thumbnailbarwidget.h \
    DicomViewer/imageinstance.h \
    DicomViewer/imageloadthread.h \
    DicomViewer/dicomimageview.h \
    DicomService/printscu.h \
    MainStation/printparamdialog.h \
    MainStation/studyexplorerwidget.h \
    StructuredReport/reporteditwidget.h \
    DicomViewer/imageviewwidget.h \
    MainStation/archivewidget.h \
    ImageAcquisitor/imageacquisitwidget.h \
    ImageAcquisitor/imageeditdialog.h \
    MainStation/systemsettingswidget.h \
    MainStation/dicomscpmodel.h \
    MainStation/localsettings.h \
    MainStation/timelabel.h \
    MainStation/sqlstudymodel.h \
    MainStation/sqlstudyview.h \
    MainStation/sqlimagemodel.h \
    MainStation/sqlimageview.h \
    MainStation/sqlreportmodel.h \
    MainStation/sqlreportview.h \
    share/studyrecord.h \
    MainStation/grouppermissionmodel.h \
    MainStation/grouplistview.h \
    MainStation/importstudymodel.h \
    MainStation/scandcmfilethread.h \
    MainStation/importstudyview.h \
    MainStation/importdcmfilethread.h \
    StructuredReport/mdisubwindow.h \
    ImageAcquisitor/studyregisterwidget.h \
    ImageAcquisitor/worklistitemmodel.h \
    ImageAcquisitor/newstudydialog.h \
    ImageAcquisitor/procedureitem.h \
    ImageAcquisitor/procedureitemdelegate.h \
    ImageAcquisitor/procedureitemmodel.h \
    MainStation/studydbmanager.h \
    MainStation/logdbmanager.h \
    MainStation/modifystudythread.h \
    ImageAcquisitor/VADAVIntf.h \
    ImageAcquisitor/crapi_dll.h \
    ImageAcquisitor/errors.h \
    ImageAcquisitor/structures.h \
    ImageAcquisitor/initcrdetectorthread.h \
    StructuredReport/diagnostictemplateview.h \
    StructuredReport/newitemdialog.h \
    StructuredReport/diagnostictemplatedelegate.h \
    ImageAcquisitor/IeLauncher.h \
    ImageAcquisitor/imageprocessthread.h \
    MainStation/exitwidget.h \
    ImageAcquisitor/carerayacquisitthread.h

SOURCES += \
    DicomService/echoscu.cpp \
    DicomService/findscucallback.cpp \
    DicomService/findscuthread.cpp \
    DicomService/storescuthread.cpp \
    DicomService/wlistscuthread.cpp \
    GraphicsItem/abstractpathitem.cpp \
    GraphicsItem/graphicsangleitem.cpp \
    GraphicsItem/graphicsarrowitem.cpp \
    GraphicsItem/graphicscliprectitem.cpp \
    GraphicsItem/graphicscobbangleitem.cpp \
    GraphicsItem/graphicscrossitem.cpp \
    GraphicsItem/graphicsellipseitem.cpp \
    GraphicsItem/graphicslineitem.cpp \
    GraphicsItem/graphicsrectitem.cpp \
    GraphicsItem/graphicssimpletextitem.cpp \
    GraphicsItem/graphicstextmarkdialog.cpp \
    GraphicsItem/graphicstextmarkitem.cpp \
    ImageAcquisitor/bodyprotographicsview.cpp \
    ImageAcquisitor/imageeditgraphicsview.cpp \
    MainStation/aboutdialog.cpp \
    MainStation/exportimagedialog.cpp \
    MainStation/exportimagethread.cpp \
    MainStation/itemdelegates.cpp \
    MainStation/logindialog.cpp \
    MainStation/mainwindow.cpp \
    MainStation/registerdialog.cpp \
    MainStation/resetpassworddialog.cpp \
    MainStation/sendstudydialog.cpp \
    MainStation/usergroupmodels.cpp \
    share/global.cpp \
    StructuredReport/diagnostictemplatemodel.cpp \
    StructuredReport/reportwidget.cpp \
    StructuredReport/srsettingsdialog.cpp \
    main.cpp \
    share/dicomscp.cpp \
    DicomViewer/dicomimagelabel.cpp \
    DicomViewer/thumbnailbarwidget.cpp \
    DicomViewer/imageinstance.cpp \
    DicomViewer/imageloadthread.cpp \
    DicomViewer/dicomimageview.cpp \
    DicomService/printscu.cpp \
    MainStation/printparamdialog.cpp \
    MainStation/studyexplorerwidget.cpp \
    StructuredReport/reporteditwidget.cpp \
    DicomViewer/imageviewwidget.cpp \
    MainStation/archivewidget.cpp \
    ImageAcquisitor/imageacquisitwidget.cpp \
    ImageAcquisitor/imageeditdialog.cpp \
    MainStation/systemsettingswidget.cpp \
    MainStation/dicomscpmodel.cpp \
    MainStation/localsettings.cpp \
    MainStation/timelabel.cpp \
    MainStation/sqlstudymodel.cpp \
    MainStation/sqlstudyview.cpp \
    MainStation/sqlimagemodel.cpp \
    MainStation/sqlimageview.cpp \
    MainStation/sqlreportmodel.cpp \
    MainStation/sqlreportview.cpp \
    MainStation/grouppermissionmodel.cpp \
    MainStation/grouplistview.cpp \
    MainStation/importstudymodel.cpp \
    MainStation/scandcmfilethread.cpp \
    MainStation/importstudyview.cpp \
    MainStation/importdcmfilethread.cpp \
    StructuredReport/mdisubwindow.cpp \
    ImageAcquisitor/studyregisterwidget.cpp \
    ImageAcquisitor/worklistitemmodel.cpp \
    ImageAcquisitor/newstudydialog.cpp \
    ImageAcquisitor/procedureitemdelegate.cpp \
    ImageAcquisitor/procedureitemmodel.cpp \
    ImageAcquisitor/procedureitem.cpp \
    MainStation/studydbmanager.cpp \
    MainStation/logdbmanager.cpp \
    MainStation/modifystudythread.cpp \
    ImageAcquisitor/VADAVIntf.cpp \
    ImageAcquisitor/initcrdetectorthread.cpp \
    StructuredReport/diagnostictemplateview.cpp \
    StructuredReport/newitemdialog.cpp \
    StructuredReport/diagnostictemplatedelegate.cpp \
    ImageAcquisitor/imageprocessthread.cpp \
    MainStation/exitwidget.cpp \
    ImageAcquisitor/carerayacquisitthread.cpp

FORMS += \
    GraphicsItem/graphicstextmarkdialog.ui \
    MainStation/aboutdialog.ui \
    MainStation/exportimagedialog.ui \
    MainStation/logindialog.ui \
    MainStation/mainwindow.ui \
    MainStation/registerdialog.ui \
    MainStation/resetpassworddialog.ui \
    MainStation/sendstudydialog.ui \
    StructuredReport/reportwidget.ui \
    StructuredReport/srsettingsdialog.ui \
    MainStation/printparamdialog.ui \
    MainStation/studyexplorerwidget.ui \
    StructuredReport/reporteditwidget.ui \
    DicomViewer/imageviewwidget.ui \
    MainStation/archivewidget.ui \
    ImageAcquisitor/imageacquisitwidget.ui \
    ImageAcquisitor/imageeditdialog.ui \
    MainStation/systemsettingswidget.ui \
    ImageAcquisitor/studyregisterwidget.ui \
    ImageAcquisitor/newstudydialog.ui \
    StructuredReport/newitemdialog.ui \
    MainStation/exitwidget.ui


