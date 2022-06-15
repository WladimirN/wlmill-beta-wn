#include "wlheightmapwidget.h"
#include "ui_wlheightmapwidget.h"

WLHeightMapWidget::WLHeightMapWidget(WLHeightMap *_heightMap,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLHeightMapWidget)
{
    ui->setupUi(this);

    m_heightMap=_heightMap;

    m_heightMapTModel = new WLHeightMapTableModel(m_heightMap,this);

    ui->tableView->setModel(m_heightMapTModel);

    ui->sbStepInter->setValue(m_heightMap->getInterpStepX());

    ui->cbType->addItems(QStringList()<<"Bicubic (U)"<<"Bilenear(V)");

    ui->cbType->setCurrentIndex(m_heightMap->getTypeInterpoliation());

    connect(ui->cbType,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){m_heightMap->setTypeInterpoliation(static_cast<WLHeightMap::typeInterpoliation>(index));});

    connect(m_heightMap,&WLHeightMap::changed,[=](){ui->cbEnable->setEnabled(m_heightMap->isValid());});
    connect(m_heightMap,&WLHeightMap::changedEnable,ui->cbEnable,&QCheckBox::setChecked);

    connect(m_heightMap,&WLHeightMap::changedElement,[=](int x,int y)
       {
       ui->tableView->update(m_heightMapTModel->index( m_heightMap->countY()-1-y,x));
       ui->cbEnable->setEnabled(m_heightMap->isValid());
       });

}

WLHeightMapWidget::~WLHeightMapWidget()
{
delete ui;
}


void WLHeightMapWidget::on_pbLoad_clicked()
{

}

void WLHeightMapWidget::on_cbEnable_stateChanged(int arg1)
{
m_heightMap->setEnable(arg1);
}

void WLHeightMapWidget::on_cbShow_stateChanged(int arg1)
{
m_heightMap->setShow(arg1);
}

void WLHeightMapWidget::on_cbShowGrid_stateChanged(int arg1)
{
m_heightMap->setShowGrid(arg1);
}

void WLHeightMapWidget::on_sbStepInter_valueChanged(double arg1)
{
m_heightMap->setInterpStepX(arg1);
m_heightMap->setInterpStepY(arg1);
}
