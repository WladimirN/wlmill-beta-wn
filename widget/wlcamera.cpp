#include "wlcamera.h"
#include "ui_wlcamera.h"

#include <QPainter>

WLCamera::WLCamera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLCamera)
{
    ui->setupUi(this);    

    findCameras();

    connect(ui->cbCamera,&QComboBox::currentTextChanged,this,&WLCamera::setCameraDescription);

    setCamera(QCameraInfo::defaultCamera());

    connect(ui->pbRefresh,&QPushButton::clicked,this,&WLCamera::findCameras);
   // ui->viewCamera->close();
  //  QTimer::singleShot(1000,this,&WLCamera::takeImage);
}

WLCamera::~WLCamera()
{
    delete ui;
}

void WLCamera::findCameras()
{
const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();

ui->cbCamera->clear();

for (const QCameraInfo &cameraInfo : availableCameras) {
    ui->cbCamera->addItem(cameraInfo.description());
    }
}

void WLCamera::takeImage()
{
m_isCapturingImage = true;
m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
m_imageCapture->capture();
}

void WLCamera::setCameraDescription(QString descript)
{
const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();

for (const QCameraInfo &cameraInfo : availableCameras) {
      if(cameraInfo.description()==descript){
                       setCamera(cameraInfo);
                       return;
                }
      }

setCamera(QCameraInfo::defaultCamera());
}

void WLCamera::processCapturedImage(int requestId, const QImage &img)
{
Q_UNUSED(requestId);
QImage scaledImage = img.scaled(ui->viewCamera->size(),
                                Qt::KeepAspectRatio,
                                Qt::SmoothTransformation);

QPixmap pixmap=QPixmap::fromImage(scaledImage);

QPainter painter(&pixmap);

painter.drawLine(0,pixmap.height()/2,pixmap.width(),pixmap.height()/2);
painter.drawLine(pixmap.width()/2,0,pixmap.width()/2,pixmap.height());

//ui->label->setPixmap(pixmap);

// Display captured image for 4 seconds.
//displayCapturedImage();
//QTimer::singleShot(4000, this, &Camera::displayViewfinder);
//m_isCapturingImage = false;
//if (m_applicationExiting)
//    close();
//else
    QTimer::singleShot(0, this, &WLCamera::takeImage);
}

void WLCamera::setCamera(const QCameraInfo &cameraInfo)
{
qDebug()<<"WLCamera::setCamera"<<cameraInfo.deviceName();

m_camera.reset(new QCamera(cameraInfo));

//connect(m_camera.data(), &QCamera::stateChanged, this, &Camera::updateCameraState);
//connect(m_camera.data(), QOverload<QCamera::Error>::of(&QCamera::error), this, &Camera::displayCameraError);

m_mediaRecorder.reset(new QMediaRecorder(m_camera.data()));
//connect(m_mediaRecorder.data(), &QMediaRecorder::stateChanged, this, &Camera::updateRecorderState);


m_imageCapture.reset(new QCameraImageCapture(m_camera.data()));

connect(m_imageCapture.data(), &QCameraImageCapture::imageCaptured, this, &WLCamera::processCapturedImage);

//connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this, &Camera::updateRecordTime);
//connect(m_mediaRecorder.data(), QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error),
//        this, &Camera::displayRecorderError);

//m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));

//connect(ui->exposureCompensation, &QAbstractSlider::valueChanged, this, &Camera::setExposureCompensation);

ui->viewCamera->setAspectRatioMode(Qt::IgnoreAspectRatio);
m_camera->setViewfinder(ui->viewCamera);

connect(m_imageCapture.data(), &QCameraImageCapture::imageCaptured,[=](int id, const QImage &preview)
{
qDebug()<<"id="<<id<<preview.width()<<"x"<<preview.height();

});
/*
updateCameraState(m_camera->state());
updateLockStatus(m_camera->lockStatus(), QCamera::UserRequest);
updateRecorderState(m_mediaRecorder->state());
*/
//connect(m_imageCapture.data(), &QCameraImageCapture::readyForCaptureChanged, this, &Camera::readyForCapture);
//connect(m_imageCapture.data(), &QCameraImageCapture::imageCaptured, this, &Camera::processCapturedImage);
//connect(m_imageCapture.data(), &QCameraImageCapture::imageSaved, this, &Camera::imageSaved);
connect(m_imageCapture.data(), QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
        this, &WLCamera::displayCaptureError);
/*
connect(m_camera.data(), QOverload<QCamera::LockStatus, QCamera::LockChangeReason>::of(&QCamera::lockStatusChanged),
        this, &Camera::updateLockStatus);

ui->captureWidget->setTabEnabled(0, (m_camera->isCaptureModeSupported(QCamera::CaptureStillImage)));
ui->captureWidget->setTabEnabled(1, (m_camera->isCaptureModeSupported(QCamera::CaptureVideo)));

updateCaptureMode();
*/
m_camera->start();

ui->cbResolution->clear();
ui->cbResolution->addItem(tr("Default Resolution"));
const QList<QSize> supportedResolutions = m_imageCapture.data()->supportedResolutions();

for (const QSize &resolution : supportedResolutions) {
    ui->cbResolution->addItem(QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
                                    QVariant(resolution));
}

connect(ui->cbResolution,&QComboBox::currentTextChanged,[=](){
  QImageEncoderSettings settings = m_imageCapture.data()->encodingSettings();
  settings.setResolution(ui->cbResolution->itemData(ui->cbResolution->currentIndex()).toSize());

  QSize resolution=settings.resolution();

  qDebug()<<"set Resolutions"<<QString("%1x%2").arg(resolution.width()).arg(resolution.height());
  m_imageCapture.data()->setEncodingSettings(settings);
});
}

void WLCamera::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
   // m_isCapturingImage = false;
}
