#ifndef SRSETTINGSDIALOG_H
#define SRSETTINGSDIALOG_H

#include <QDialog>

#define REPORT_TITLE_TEXT "REPORTTITLETEXT"
#define REPORT_TITLE_FONT "REPORTTITLEFONT"
#define INSTITUTION_NAME_FONT "INSTITUTIONNAMEFONT"
#define PATIENT_INFO_FONT "PATIENTINFOFONT"
#define REPORT_TEXT_FONT "REPORTTEXTFONT"
#define PHYSICIAN_INFO_FONT "PHYSICIANINFOFONT"
#define FOOTNOTE_TEXT "FOOTNOTETEXT"
#define FOOTNOTE_FONT "FOOTNOTEFOOT"
#define IMAGE_WIDTH "IMAGEWIDTH"
#define IMAGE_HEIGHT "IMAGEHEIGHT"

namespace Ui {
class SRSettingsDialog;
}

class SRSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SRSettingsDialog(QWidget *parent = 0);
    ~SRSettingsDialog();

protected slots:
    void setInstitutionNameFont();
    void setReportTitleFont();
    void setPatientInfoFont();
    void setReportTextFont();
    void setPhysicianInfoFont();
    void setFootNoteFont();

    void onOk();

private:
    void init();
    Ui::SRSettingsDialog *ui;
};

#endif // SRSETTINGSDIALOG_H
