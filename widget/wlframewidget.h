#ifndef WLFRAMEWIDGET_H
#define WLFRAMEWIDGET_H

#include <QDialog>
#include "ui_wlframewidget.h"
#include "WLFrame.h"

class WLFrameWidget : public QDialog
{
	Q_OBJECT

public:
    WLFrameWidget(QWidget *parent = nullptr);
	~WLFrameWidget();

	void setFramen(WLFramen Framen);
	void setFrame(WLFrame Frame);

    WLFramen getFramen() {return curFramen;}

private:
	Ui::WLFrameWidget ui;
	
	WLFramen curFramen;
	WLFramen baseFramen;

signals:
	void changedFrame(WLFrame);
	void changedFramen(WLFramen);

private slots:
    void updateFrameData();

	void onButtonFromStr();
	void onButtonOk()     {done(1); updateFrameData();}	
    void onButtonCancel();
};

#endif // WLFRAMEWIDGET_H
