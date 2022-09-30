#ifndef WLDATAWIDGET_H
#define WLDATAWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wldatatablemodel.h"
#include "wltbuttonscript.h"

namespace Ui {
class WLDataWidget;
}

class WLDataWidget;

class WLTBarData: public WLTBarScript
{
Q_OBJECT

public:
    explicit WLTBarData(WLEVScript *_script,QWidget *parent=nullptr):WLTBarScript(_script,"",parent)
    {

    }
    void setTWidget(WLDataWidget *_DataWidget);

    Q_INVOKABLE     int curSelect();
    Q_INVOKABLE QString curData();

private:
    WLDataWidget *DataWidget=nullptr;
};

class WLDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WLDataWidget(WLGCode *_GCode,QWidget *parent = nullptr);
    ~WLDataWidget();

    void setModel(WLDataTableModel *Model);

    void setHeadersTable(QStringList headers);
    QStringList getHeaderTable();

    void addToolBar(WLTBarData *toolBar);

        int curIndexTool();
    QString curRowTool();
private:
    Ui::WLDataWidget *ui;

    WLGCode *m_GCode;
    WLDataTableModel *m_Model=nullptr;

private slots:

};

#endif // WLDATAWIDGET_H


