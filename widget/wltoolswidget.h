#ifndef WLTOOLSWIDGET_H
#define WLTOOLSWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wlgtoolstablemodel.h"

namespace Ui {
class WLToolsWidget;
}

class WLToolsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WLToolsWidget(WLGCode *_GCode,QWidget *parent = nullptr);
    ~WLToolsWidget();

    void setHeadersTable(QStringList headers);
    QStringList getHeaderTable();
private:
    Ui::WLToolsWidget *ui;

    WLGCode *m_GCode;
    WLGToolsTableModel *Model=nullptr;

private slots:

};

#endif // WLTOOLSWIDGET_H


