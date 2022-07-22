#ifndef WLCAMERA_H
#define WLCAMERA_H

#include <QWidget>
#include <QComboBox>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QMessageBox>
#include <QTimer>
#include <QMediaRecorder>

namespace Ui {
class WLCamera;
}

class WLCamera : public QWidget
{
    Q_OBJECT

public:
    explicit WLCamera(QWidget *parent = nullptr);
    ~WLCamera();

private:
    Ui::WLCamera *ui;

     QScopedPointer<QCamera> m_camera;
     QScopedPointer<QCameraImageCapture> m_imageCapture;
     QScopedPointer<QMediaRecorder> m_mediaRecorder;

     bool m_isCapturingImage = false;
private:
    void updateResolution();    

private slots:
    void findCameras();
    void takeImage();
    void setCamera(const QCameraInfo &cameraInfo);
    void setCameraDescription(QString descript);
    void processCapturedImage(int requestId, const QImage &img);

    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);
};

#endif // WLCAMERA_H
