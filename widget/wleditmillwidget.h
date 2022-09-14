#ifndef WLEDITMILLWIDGET_H
#define WLEDITMILLWIDGET_H

#include <QWidget>
#include <QSerialPortInfo>

#include "wlmillmachine.h"
#include "ui_wleditmillwidget.h"


class WLEditMillWidget : public QDialog
{
	Q_OBJECT

public:
    WLEditMillWidget(WLGMachine *_MillMachine,QWidget *parent = nullptr);
	~WLEditMillWidget();

void addTabWidget(QDialog *dialog,QString name)              {ui.tabWidget->addTab(dialog,name); dialogList+=dialog;}
void insertTabWidget(int index,QDialog *dialog,QString name) {ui.tabWidget->insertTab(index,dialog,name);}


bool saveDataMill();

QString verifyError();

bool getNeedClose() const;

private:
WLGMachine *MillMachine;

QList <QDialog*> dialogList;

	Ui::WLEditMillWidget ui;

    bool m_needClose=false;

private slots:
	void onVerifyError();

    // QDialog interface
public slots:
    void accept();

    // QWidget interface    
protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // WLEDITMILLWIDGET_H
