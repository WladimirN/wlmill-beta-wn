#ifndef WLCHOSCPWIDGET_H
#define WLCHOSCPWIDGET_H

#include <QWidget>
#include "wlmotion.h"
#include "wlmoduleoscp.h"

namespace Ui {
class WLChOscpWidget;
}

class WLChOscpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WLChOscpWidget(WLModuleOscp *module,quint8 iCh,QWidget *parent = nullptr);

    ~WLChOscpWidget();

    bool isChecked();    
private:
    Ui::WLChOscpWidget *ui;

private:
    QList <WLSrcChOscp> src;

    WLModuleOscp *m_module=nullptr;
    quint8 m_iCh=0;

private slots:
    void updateIElement();
    void updateElement();
    void updateTypeData();
    void updateSource();

public slots:
   void setSource(QList<WLSrcChOscp> _src);
   void setColor(QColor);
};


#endif // WLCHOSCPWIDGET_H
