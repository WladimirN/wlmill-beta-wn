#ifndef WLTOOLSWIDGET_H
#define WLTOOLSWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wlgtoolstablemodel.h"
#include "wltbuttonscript.h"

namespace Ui {
class WLToolsWidget;
}

class WLToolsWidget;

class WLTBarTools: public WLTBarScript
{
Q_OBJECT

public:
    explicit WLTBarTools(WLEVScript *_script,QWidget *parent=nullptr):WLTBarScript(_script,"",parent)
    {

    }
    void setTWidget(WLToolsWidget *_toolsWidget);

    Q_INVOKABLE     int selectTool();
    Q_INVOKABLE QString selectData();

private:
    WLToolsWidget *toolsWidget=nullptr;
};

class WLToolsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WLToolsWidget(WLGCode *_GCode,QWidget *parent = nullptr);
    ~WLToolsWidget();

    void setHeadersTable(QStringList headers);
    QStringList getHeaderTable();

    void addToolBar(WLTBarTools *toolBar);

        int curIndexTool();
    QString curRowTool();
private:
    Ui::WLToolsWidget *ui;

    WLGCode *m_GCode;
    WLGToolsTableModel *m_Model=nullptr;

private slots:

};

#endif // WLTOOLSWIDGET_H


