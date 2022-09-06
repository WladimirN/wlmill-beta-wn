#ifndef WLCAMERAVIEW_H
#define WLCAMERAVIEW_H

#include <QObject>
#include <QCameraViewfinder>

class WLCameraView : public QCameraViewfinder
{
public:
    explicit WLCameraView(QWidget *parent = nullptr);

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // WLCAMERAVIEW_H
