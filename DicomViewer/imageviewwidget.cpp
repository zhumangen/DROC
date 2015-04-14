#include "imageviewwidget.h"
#include "ui_imageviewwidget.h"

#include "../share/configfiles.h"
#include "../share/dicomscp.h"
#include "thumbnailbarwidget.h"
#include "dicomimageview.h"
#include "dicomimagelabel.h"
#include "imageinstance.h"
#include "../DicomService/printscu.h"
#include "../MainStation/mainwindow.h"

#include "dcmtk/ofstd/ofcond.h"

#include <QFile>
#include <QTextStream>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>

ImageViewWidget::ImageViewWidget(QWidget *parent) :
    currentView(0),
    viewMaximised(0),
    thumbnailBar(new ThumbnailBarWidget),
    QWidget(parent),
    ui(new Ui::ImageViewWidget)
{
    ui->setupUi(this);
    init();
}

ImageViewWidget::~ImageViewWidget()
{
    clearPrintScps();
    delete ui;
}

void ImageViewWidget::init()
{
    loadPrinterConfig();

    ui->thumbnailScrollArea->setWidget(thumbnailBar);

    viewLayout = new QGridLayout(ui->viewContainer);
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(0);

    //onFilmLayoutChanged(ui->filmLayoutCombo->currentText());
    createConnections();
}

void ImageViewWidget::setImageFiles(const QStringList &files)
{
    clear();
    thumbnailBar->setImageFiles(files);
}

bool ImageViewWidget::getCurrentStudy(StudyRecord &study)
{
    return currentView && currentView->getCurrentStudy(study);
}

DicomImageView *ImageViewWidget::createImageView()
{
    DicomImageView *v = new DicomImageView;
    connect(v, SIGNAL(viewClicked(DicomImageView*)),
            this, SLOT(onViewClicked(DicomImageView*)));
    connect(v, SIGNAL(viewDoubleclicked(DicomImageView*)),
            this, SLOT(onViewDoubleClicked(DicomImageView*)));
    return v;
}

void ImageViewWidget::setViewPrintMode(bool yes)
{
    foreach (DicomImageView *v, viewList) {
        v->setViewMode(yes?DicomImageView::VM_Print:DicomImageView::VM_Observe);
    }
}

void ImageViewWidget::loadPrinterConfig()
{
    QFile file(PRINTERS_CFG);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString line;
        while (!in.atEnd()) {
            line = in.readLine();
            if (line.startsWith(QChar('#'))) continue;
            int index = line.indexOf(QChar('='));
            if (index <= 0) continue;
            QString tag = line.mid(0, index).remove(QChar(' '));
            QString value = line.mid(index+1).remove(QChar(' '));
            if (tag.isEmpty() || value.isEmpty()) continue;
            tag = tag.toUpper();
            if (tag == "FILMSIZEID") {
                ui->filmSizeCombo->addItems(value.split(QChar('\\')));
            } else if (tag == "DISPLAYFORMAT") {
                value.replace(QChar(','), " X ");
                ui->filmLayoutCombo->addItems(value.split(QChar('\\')));
            }
        }
        file.close();
    }
}

void ImageViewWidget::createConnections()
{
    connect(ui->observeModeRadio, SIGNAL(toggled(bool)),
            this, SLOT(onModeChanged(bool)));
    connect(ui->printerNameCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onPrinterChanged(int)));
    connect(ui->filmLayoutCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(onFilmLayoutChanged(QString)));
    connect(ui->filmSizeCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(onFilmSizeChanged(QString)));
    connect(ui->filmOrientationCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onFilmOrientationChanged(int)));
    connect(thumbnailBar, SIGNAL(imageInserted(ImageInstance*)),
            this, SLOT(onImageInserted(ImageInstance*)));
    connect(thumbnailBar, SIGNAL(imageDoubleClicked(DicomImageLabel*)),
            this, SLOT(onImageDoubleClicked(DicomImageLabel*)));
    connect(thumbnailBar, SIGNAL(currentChanged(DicomImageLabel*)),
            this, SLOT(onImageChanged(DicomImageLabel*)));

    connect(ui->zoomButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->panButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->rotateCCWButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->rotateCWButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->hFllipButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->vFlipButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->roiButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->negativeButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->cursorButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->lengthButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->angleButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->cobbAngleButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->ellipseButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->rectangleButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->textButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->arrowButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->itemDeleteButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(toolButtonClicked()));
    connect(ui->rawImageButton, SIGNAL(clicked(bool)), this, SLOT(onRawImageButton(bool)));
    connect(ui->filmPrintButton, SIGNAL(clicked()), this, SLOT(onFilmPrint()));
    connect(ui->windowsPrintButton, SIGNAL(clicked()), this, SLOT(onWindowsPrint()));
}

void ImageViewWidget::onModeChanged(bool obsMode)
{
    if (obsMode) {
        ui->printNameLabel->setDisabled(true);
        ui->printerNameCombo->setDisabled(true);
        ui->filmSizeLabel->setDisabled(true);
        ui->filmSizeCombo->setDisabled(true);
        ui->copiesSpin->setDisabled(true);
        ui->copiesLabel->setDisabled(true);
        ui->filmPrintButton->setDisabled(true);
        ui->windowsPrintButton->setDisabled(true);
        viewLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        ui->printNameLabel->setDisabled(false);
        ui->printerNameCombo->setDisabled(false);
        ui->filmSizeLabel->setDisabled(false);
        ui->filmSizeCombo->setDisabled(false);
        ui->copiesSpin->setDisabled(false);
        ui->copiesLabel->setDisabled(false);
        if (mainWindow->getCurrentGroup().permissions & GP_PrintImage) {
            if (ui->printerNameCombo->count())
                ui->filmPrintButton->setDisabled(false);
            ui->windowsPrintButton->setDisabled(false);
        }
        onFilmSizeChanged(ui->filmSizeCombo->currentText());
    }
    setViewPrintMode(!obsMode);
}

void ImageViewWidget::onViewClicked(DicomImageView *view)
{
    if (currentView) currentView->setHighlight(false);
    currentView = view;
    if (currentView) {
        currentView->setHighlight(true);
        thumbnailBar->setCurrentImageLabel(currentView->getImageInstance());
        ui->thumbnailScrollArea->ensureWidgetVisible(thumbnailBar->getCurrentImageLabel());
        emit currentViewChanged(currentView);
    }

    updateRawImageButton();
}

void ImageViewWidget::onViewDoubleClicked(DicomImageView *view)
{
    if (currentView) currentView->setHighlight(false);
    currentView = view;
    if (currentView) {
        currentView->setHighlight(true);
        thumbnailBar->setCurrentImageLabel(currentView->getImageInstance());
        ui->thumbnailScrollArea->ensureWidgetVisible(thumbnailBar->getCurrentImageLabel());
        emit currentViewChanged(currentView);
        if (ui->observeModeRadio->isChecked()) {
            if (viewMaximised) {
                foreach (DicomImageView *v, viewList) {
                    v->setVisible(true);
                }
                viewMaximised = false;
            } else {
                foreach (DicomImageView *v, viewList) {
                    v->setVisible(false);
                }
                view->setVisible(true);
                viewMaximised = true;
            }
        }
    }
}

void ImageViewWidget::onImageInserted(ImageInstance *image)
{
    foreach (DicomImageView *v, viewList) {
        if (!v->hasImage()) {
            v->setImageInstance(image);
            if (currentView == v) {
                thumbnailBar->setCurrentImageLabel(v->getImageInstance());
                updateRawImageButton();
            }
            return;
        }
    }

    if (viewList.size() >= 4) return;

    DicomImageView *view = createImageView();
    view->setImageInstance(image);
    if (viewMaximised) view->setVisible(false);
    if (ui->printModeRadio->isChecked()) view->setViewMode(DicomImageView::VM_Print);
    viewList.append(view);

    int index = ui->filmLayoutCombo->currentIndex()+1;
    while (index < ui->filmLayoutCombo->count()) {
        int col, row;
        if (PrintParam::parseDisplayFormat(ui->filmLayoutCombo->itemText(index), col, row) &&
                (col*row >= viewList.size())) {
            ui->filmLayoutCombo->setCurrentIndex(index);
            break;
        }
        index++;
    }
}

void ImageViewWidget::onImageDoubleClicked(DicomImageLabel *label)
{
    if (currentView) {
        currentView->setImageInstance(label->getImageInstance());
        updateRawImageButton();
    }
}

void ImageViewWidget::onImageChanged(DicomImageLabel *label)
{
    ui->thumbnailScrollArea->ensureWidgetVisible(label);
}

void ImageViewWidget::onPrinterChanged(int /*index*/)
{
    PrintScp *printer = static_cast<PrintScp*>((void*)ui->printerNameCombo->currentData().toULongLong());
    if (printer) {
        ui->filmSizeCombo->setCurrentText(printer->param.filmSizeId);
        ui->filmOrientationCombo->setCurrentText(printer->param.filmOrientation);
    }
}

void ImageViewWidget::onFilmLayoutChanged(const QString &layout)
{
    int col, row;
    if (!PrintParam::parseDisplayFormat(layout, col, row))
        return;

    if (viewMaximised) onViewDoubleClicked(currentView);

    if (ui->filmOrientationCombo->currentText() == "LANDSCAPE") {
        int temp = row;
        row = col;
        col = temp;
    }

    int cellWidth = viewLayout->contentsRect().width() / col;
    int cellHeight = viewLayout->contentsRect().height() / row;
    foreach (DicomImageView *v, viewList) {
        viewLayout->removeWidget(v);
    }

    int viewCount = col * row;
    while (viewCount < viewList.size()) {
        DicomImageView *v = viewList.takeLast();
        if (currentView == v) currentView = 0;
        delete v;
    }

    DicomImageView *view;
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            if (i * col + j < viewList.size()) {
                view = viewList[i * col + j];
                view->setSizeHint(QSize(cellWidth, cellHeight));
            } else {
                view = createImageView();
                view->setSizeHint(QSize(cellWidth, cellHeight));
                if (viewMaximised) view->setVisible(false);
                viewList.append(view);
            }
            viewLayout->addWidget(view, i, j);
        }
    }

    if ((!currentView) && viewList.size())
        onViewClicked(viewList.first());
}

void ImageViewWidget::onFilmOrientationChanged(int /*index*/)
{
    if (ui->printModeRadio->isChecked())
        onFilmSizeChanged(ui->filmSizeCombo->currentText());
    onFilmLayoutChanged(ui->filmLayoutCombo->currentText());
}

void ImageViewWidget::onFilmSizeChanged(const QString &sizeId)
{
    double ratio;
    if (PrintParam::parseFilmSizeRatio(sizeId, ratio)) {
        int margin;
        QSize containerRect = ui->viewContainer->size();

        if (ui->filmOrientationCombo->currentText() == "LANDSCAPE")
            ratio = 1 / ratio;
        if (containerRect.width() > containerRect.height() * ratio) {
            margin = (containerRect.width()-containerRect.height()*ratio)/2;
            viewLayout->setContentsMargins(margin, 0, margin, 0);
        } else {
            margin = (containerRect.height()-containerRect.width()/ratio)/2;
            viewLayout->setContentsMargins(0, margin, 0, margin);
        }
    }
}

void ImageViewWidget::setObserveMode(bool yes)
{
    if (yes) {
        ui->observeModeRadio->setChecked(true);
    } else {
        ui->printModeRadio->setChecked(true);
    }
}

void ImageViewWidget::clear()
{
    ui->filmLayoutCombo->setCurrentIndex(0);
    foreach (DicomImageView *v, viewList) {
        v->clear();
        v->reset();
        v->setVisible(true);
    }
    viewMaximised = false;
    thumbnailBar->clear();
}

void ImageViewWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    if (ui->printModeRadio->isChecked())
        onFilmSizeChanged(ui->filmSizeCombo->currentText());
}

void ImageViewWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Home:
        if (currentView && (currentView != viewList.first())) {
            currentView->setHighlight(false);
            currentView = viewList.first();
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        } else if ((!currentView) && viewList.size()) {
            currentView = viewList.first();
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        }
        break;
    case Qt::Key_PageDown:
    case Qt::Key_Down:
    case Qt::Key_Right:
        if (currentView && (viewList.size()>1)) {
            currentView->setHighlight(false);
            if (currentView == viewList.last()) currentView = viewList.first();
            else currentView = viewList.at(viewList.indexOf(currentView)+1);
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        } else if (viewList.size()) {
            currentView = viewList.first();
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        }
        break;
    case Qt::Key_End:
        if (currentView && (currentView != viewList.last())) {
            currentView->setHighlight(false);
            currentView = viewList.last();
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        } else if ((!currentView) && viewList.size()) {
            currentView = viewList.last();
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        }
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
    case Qt::Key_Left:
        if (currentView && (viewList.size()>1)) {
            currentView->setHighlight(false);
            if (currentView == viewList.first()) currentView = viewList.last();
            else currentView = viewList.at(viewList.indexOf(currentView)-1);
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        } else if (viewList.size()) {
            currentView = viewList.last();
            currentView->setHighlight(true);
            emit currentViewChanged(currentView);
        }
        break;
    default:
        QWidget::keyPressEvent(e);
    }
}

void ImageViewWidget::toolButtonClicked()
{
    if (currentView) {
        QToolButton *b = qobject_cast<QToolButton*>(sender());
        if (b == ui->zoomButton) {
            foreach (DicomImageView *v, viewList) {
                v->zoomImage();
            }
        } else if (b == ui->panButton) {
            foreach (DicomImageView *v, viewList) {
                v->panImage();
            }
        } else if (b == ui->rotateCCWButton) {
            currentView->rotateCCW();
        } else if (b == ui->rotateCWButton) {
            currentView->rotateCW();
        } else if (b == ui->hFllipButton) {
            currentView->hFlip();
        } else if (b == ui->vFlipButton) {
            currentView->vFlip();
        } else if (b == ui->roiButton) {
            foreach (DicomImageView *v, viewList) {
                v->setRoiWindow();
            }
        } else if (b == ui->negativeButton) {
            currentView->inverseImage();
        } else if (b == ui->cursorButton) {
            foreach (DicomImageView *v, viewList) {
                v->setNone();
            }
        } else if (b == ui->lengthButton) {
            currentView->drawLenght();
        } else if (b == ui->angleButton) {
            currentView->drawAngle();
        } else if (b == ui->cobbAngleButton) {
            currentView->drawCobbAngle();
        } else if (b == ui->ellipseButton) {
            currentView->drawEllipse();
        } else if (b == ui->rectangleButton) {
            currentView->drawRectangle();
        } else if (b == ui->textButton) {
            currentView->drawText();
        } else if (b == ui->arrowButton) {
            currentView->drawArrow();
        } else if (b == ui->itemDeleteButton) {
            currentView->removeCurrent();
        } else if (b == ui->resetButton) {
            currentView->reset();
        }
    }
}

void ImageViewWidget::onRawImageButton(bool raw)
{
    currentView->setImageType(raw);
    currentView->refreshViewPort();
    ui->rawImageButton->setText(raw?tr("Pr. Image"):tr("Raw Image"));
}

void ImageViewWidget::onFilmPrint()
{
    PrintScp *printer = static_cast<PrintScp*>((void*)ui->printerNameCombo->currentData().toULongLong());
    if (!printer) return;
    printer->param.displayFormat = ui->filmLayoutCombo->currentText();
    printer->param.filmSizeId = ui->filmSizeCombo->currentText();
    printer->param.filmOrientation = ui->filmOrientationCombo->currentText();

    OFCondition result = printscu(viewList, *printer, ui->copiesSpin->value());

    if (EC_Normal == result)
        QMessageBox::information(this, tr("Film Print"), tr("Film Print Succeeded."));
    else
        QMessageBox::critical(this, tr("Film Print"), tr("Film Print Failed: %1.").arg(result.text()));

}

void ImageViewWidget::onWindowsPrint()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        double ratio;
        int col, row;
        if (PrintParam::parseFilmSizeRatio(ui->filmSizeCombo->currentText(), ratio) &&
                PrintParam::parseDisplayFormat(ui->filmLayoutCombo->currentText(), col, row)) {
            if (ui->filmOrientationCombo->currentIndex() == 1) {
                printer.setOrientation(QPrinter::Landscape);
                ratio = 1 / ratio;
                int temp = col;
                col = row;
                row = temp;
            } else printer.setOrientation(QPrinter::Portrait);
            printer.setCopyCount(ui->copiesSpin->value());

            QPainter painter(&printer);
            QRect viewRect = painter.viewport();

            if (viewRect.width() > viewRect.height()*ratio) {
                int margin = (viewRect.width() - viewRect.height()*ratio) / 2;
                viewRect.adjust(margin, 0, -margin, 0);
            } else {
                int margin = (viewRect.height() - viewRect.width()/ratio) / 2;
                viewRect.adjust(0, margin, 0, -margin);
            }
            painter.setViewport(viewRect);
            painter.setWindow(0, 0, viewRect.width(), viewRect.height());

            int cellWidth = viewRect.width() / col;
            int cellHeight = viewRect.height() / row;
            for (int i = 0; i < row; ++i) {
                for (int j = 0; j < col; ++j) {
                    QRect imageRect(j*cellWidth+(j==0?0:1), i*cellHeight+(i==0?0:1), cellWidth, cellHeight);
                    QPixmap pixmap = viewList[i*col+j]->getHardCopyPixmap();
                    if (!pixmap.isNull())
                        painter.drawPixmap(imageRect.topLeft(), pixmap.scaled(imageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
        } else {
            QMessageBox::critical(this, tr("Normal Print"), tr("Invalid layout or aspect ratio."));
        }
    }
}

void ImageViewWidget::clearPrintScps()
{
    for (int i = 0; i < ui->printerNameCombo->count(); ++i) {
        delete reinterpret_cast<DicomScp*>(ui->printerNameCombo->itemData(i).toULongLong());
    }
    ui->printerNameCombo->clear();
}

void ImageViewWidget::updateRawImageButton()
{
    ui->rawImageButton->setEnabled(currentView && currentView->hasRawImage());
    ui->rawImageButton->setText((currentView && currentView->isRawImage())?tr("Pr. Image"):tr("Raw Image"));
    ui->rawImageButton->setChecked(currentView && currentView->isRawImage());
}

void ImageViewWidget::onPrintScpUpdated(const QList<DicomScp *> &scps)
{
    clearPrintScps();

    foreach (DicomScp *scp, scps) {
        DicomScp *newScp = new DicomScp(*scp);
        ui->printerNameCombo->addItem(newScp->id, (qulonglong)newScp);
    }
}
