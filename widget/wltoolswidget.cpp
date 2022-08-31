#include <QMessageBox>
#include "wltoolswidget.h"
#include "ui_wltoolswidget.h"

WLToolsWidget::WLToolsWidget(WLGCode *_GCode,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLToolsWidget)
{
    ui->setupUi(this);

    m_GCode=_GCode;

    m_Model = new WLGToolsTableModel(m_GCode,this);

    ui->tableViewTools->setModel(m_Model);

    connect(ui->tableViewTools->itemDelegate(),&QAbstractItemDelegate::closeEditor,[=](){

    foreach(QModelIndex index,ui->tableViewTools->selectionModel()->selection().indexes()) {
        m_Model->setData(index,m_Model->data(ui->tableViewTools->selectionModel()->currentIndex(),Qt::EditRole),Qt::EditRole);
        }
    });
}

WLToolsWidget::~WLToolsWidget()
{
    delete ui;
}

void WLToolsWidget::setHeadersTable(QStringList headers)
{
    m_Model->setHeaders(headers);
}

QStringList WLToolsWidget::getHeaderTable()
{
    return m_Model->getHeaders();
}

void WLToolsWidget::addToolBar(WLTBarTools *toolBar)
{
    toolBar->setTWidget(this);
    ui->vLayout->addWidget(toolBar,1);
}

int WLToolsWidget::curIndexTool()
{
return m_GCode->getTools()->getKeyAt(ui->tableViewTools->currentIndex().row());
}

QString WLToolsWidget::curRowTool()
{
return m_Model->getHeaders().at(ui->tableViewTools->currentIndex().column());
}

void WLTBarTools::setTWidget(WLToolsWidget *_toolsWidget)
{
 toolsWidget=_toolsWidget;
}

int WLTBarTools::selectTool()
{

return toolsWidget ? toolsWidget->curIndexTool() : -1;
}

QString WLTBarTools::selectData()
{
return toolsWidget ? toolsWidget->curRowTool() : "";
}
