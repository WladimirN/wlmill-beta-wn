#ifndef WLENTERNUM_H
#define WLENTERNUM_H

#include <QDialog>
#include <QDebug>
#include "ui_wlenternum.h"

class WLEnterNum : public QDialog
{
	Q_OBJECT

public:
    WLEnterNum(QWidget *parent = nullptr);
	~WLEnterNum();

  double getNow(){return ui.doubleSpinBox->value();}

	void setMax(double m) {ui.doubleSpinBox->setMaximum(m);}
    void setMin(double m) {ui.doubleSpinBox->setMinimum(m);}
	void setNow(double n) {ui.doubleSpinBox->setValue(n);} 

	void setMinMaxNow(double mi,double ma,double n) {setMin(mi);setMax(ma);setNow(n);}

    void setRange(double min,double max) {ui.doubleSpinBox->setRange(min,max);}

	void setLabel(QString str)  {ui.label->setText(str);} 
	void setSuffix(QString str) {ui.doubleSpinBox->setSuffix(str);} 

    void setDecimals(double f)   {ui.doubleSpinBox->setDecimals(f);}
    void setSingleStep(double f) {ui.doubleSpinBox->setSingleStep(f);}

    void selectAll() {ui.doubleSpinBox->selectAll();
                      ui.doubleSpinBox->setFocus();}

private:
	Ui::WLEnterNum ui;
};

#endif // WLENTERNUM_H
