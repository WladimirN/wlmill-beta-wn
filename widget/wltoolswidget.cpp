#include <QMessageBox>
#include "wltoolswidget.h"
#include "ui_wltoolswidget.h"

WLToolsWidget::WLToolsWidget(WLGCode *_GCode,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLToolsWidget)
{
    ui->setupUi(this);

    m_GCode=_GCode;

    Model = new WLGToolsTableModel(m_GCode,this);

    ui->tableViewTools->setModel(Model);

    connect(ui->tableViewTools->itemDelegate(),&QAbstractItemDelegate::closeEditor,[=](){

    foreach(QModelIndex index,ui->tableViewTools->selectionModel()->selection().indexes()) {
        Model->setData(index,Model->data(ui->tableViewTools->selectionModel()->currentIndex(),Qt::EditRole),Qt::EditRole);
        }
    });
}

WLToolsWidget::~WLToolsWidget()
{
    delete ui;
}

void WLToolsWidget::setHeadersTable(QStringList headers)
{
    Model->setHeaders(headers);
}

QStringList WLToolsWidget::getHeaderTable()
{
    return Model->getHeaders();
}
