#ifndef WLMESSMANAGER_H
#define WLMESSMANAGER_H

#include <QTime>
#include <QMessageBox>
#include <QMutex>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QTimer>
#include <QToolButton>
#include <QStyle>
#include <QComboBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QWidgetAction>
#include <QMenu>
#include <QListWidget>
//#include "ui_wlmessmanager.h"

struct errData
{
QTime   time;
QString name;
QString mess;
int code;
int count;

};


class WLMessManager : public QToolButton
{
	Q_OBJECT

public:
    WLMessManager(QWidget *parent = nullptr);
	~WLMessManager();


private:
    //Ui::WLMessManager ui;

	QMutex mutex;
	QMutex mutexShow;
	QMutex mutexLog;

	QList <struct errData> List;

    QIcon *IconOn=nullptr;
    QIcon *IconOff=nullptr;

    //QComboBox   *comboBox=nullptr;
    QListWidget *comboBox=nullptr;
    QPushButton *pushButtonCErr=nullptr;

    QMenu *menu=nullptr;

	QPixmap pixmap;

    QTimer *flashTimer=nullptr;

    bool cur=0;
    int  count_flash=0;

    bool notify_red=false;
private:
	void updateList();

private slots:
	void clear();
	void updateFlash();

public slots:
	void  setMessage(QString,QString,int);
    void  setMessage(QString txt,int code) {setMessage(txt,"",code);}
    void  setMessage(QString txt)          {setMessage(txt,"",0);}

    //void setState(bool en) {ui.status->setEnabled(en);}

signals:
    void saveLog(QString,QString);

};

#endif // WLMESSMANAGER_H
