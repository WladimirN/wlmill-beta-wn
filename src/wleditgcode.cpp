#include "wleditgcode.h"
#include "ui_wleditgcode.h"

WLEditGCode::WLEditGCode(WLGCode *_gcode,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WLEditGCode)
{
    ui->setupUi(this);

    gcode=_gcode;

    ui->strRunProgram->setText(gcode->getStrRunProgram());
    ui->strInit->setText(gcode->getStrInit());

    ui->sbOffsetDrill->setValue(gcode->getOffsetBackLongDrill());
}

WLEditGCode::~WLEditGCode()
{
    delete ui;
}

void WLEditGCode::saveDataGCode()
{
gcode->setStrRunProgram(ui->strRunProgram->text());
gcode->setStrInit(ui->strInit->text());

gcode->setOffsetBackLongDrill(ui->sbOffsetDrill->value());
}

void WLEditGCode::accept()
{
saveDataGCode();
}
