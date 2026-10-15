#ifndef PLOTSETTINGSDIALOG_DIALOG_H
#define PLOTSETTINGSDIALOG_DIALOG_H


#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include "profiledocker.h"

class PlotSettingsDialog : public QDialog 
{
    Q_OBJECT
public:
    PlotSettingsDialog(QTabWidget *tab, QWidget *parent = nullptr);

	QTabWidget *tabWidget;
private:
    QDoubleSpinBox *dxSpinBox = nullptr;
    QDoubleSpinBox *dtSpinBox = nullptr;
    QDoubleSpinBox *dzSpinBox = nullptr;
    QSpinBox *subticksSpinBox = nullptr;
};
#endif
