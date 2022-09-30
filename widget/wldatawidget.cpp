#include <QMessageBox>
#include "wldatawidget.h"
#include "ui_wldatawidget.h"

WLDataWidget::WLDataWidget(WLGCode *_GCode,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLDataWidget)
{
    ui->setupUi(this);
    m_GCode=_GCode;
}

WLDataWidget::~WLDataWidget()
{
    delete ui;
}

void WLDataWidget::setModel(WLDataTableModel *Model)
{
m_Model = Model;

ui->tableViewData->setModel(m_Model);

connect(ui->tableViewData->itemDelegate(),&QAbstractItemDelegate::closeEditor,[=](){

foreach(QModelIndex index,ui->tableViewData->selectionModel()->selection().indexes()) {
    m_Model->setData(index,m_Model->data(ui->tableViewData->selectionModel()->currentIndex(),Qt::EditRole),Qt::EditRole);
    }
});
}

void WLDataWidget::setHeadersTable(QStringList headers)
{
if(m_Model)
   m_Model->setHeaders(headers);
}

QStringList WLDataWidget::getHeaderTable()
{
if(m_Model)
   return m_Model->getHeaders();
}

void WLDataWidget::addToolBar(WLTBarData *toolBar)
{
toolBar->setDataWidget(this);
ui->vLayout->addWidget(toolBar,1);
}

int WLDataWidget::curIndex()
{
return m_Model->getData()->getKeyAt(ui->tableViewData->currentIndex().row());
}

QString WLDataWidget::curData()
{
if(m_Model)
  return m_Model->getHeaders().at(ui->tableViewData->currentIndex().column());
else
  return "no model";
}

void WLTBarData::setDataWidget(WLDataWidget *_DataWidget)
{
 DataWidget=_DataWidget;
}

int WLTBarData::curIndex()
{
return DataWidget ? DataWidget->curIndex() : -1;
}

QString WLTBarData::curData()
{
return DataWidget ? DataWidget->curData() : "";
}
