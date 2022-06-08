#ifndef WLMPARAXISWIDGET_H
#define WLMPARAXISWIDGET_H

#include <QWidget>

#include "wlaxis.h"

namespace Ui {
class WLMParAxisWidget;
}

class WLMParAxisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WLMParAxisWidget(QWidget *parent = nullptr);
    ~WLMParAxisWidget();


      void setMPar(WLMParAxis _MPar);
WLMParAxis getMPar();

void setName(QString name);

private:

    WLMParAxis MPar;
    Ui::WLMParAxisWidget *ui;
};

#endif // WLMPARAXISWIDGET_H
