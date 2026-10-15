#include "plotsettingsdialog.h"


PlotSettingsDialog::PlotSettingsDialog(QTabWidget *tab, QWidget *parent)
    : QDialog(parent), tabWidget(tab)
{
    setWindowTitle("Plot settings");
    auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
    if (!docker)
        return;

    auto *form = new QFormLayout;

    auto dxSpinBox = new QDoubleSpinBox(this);
    dxSpinBox->setRange(0, 1000000);
	dxSpinBox->setValue(docker->dx);
    form->addRow("dx:", dxSpinBox);

    dtSpinBox = new QDoubleSpinBox(this);
    dtSpinBox->setRange(0, 1000000);
	dtSpinBox->setValue(docker->dt);
    form->addRow("dt:", dtSpinBox);

    dzSpinBox = new QDoubleSpinBox(this);
    dzSpinBox->setRange(0, 1000000);
	dxSpinBox->setValue(docker->dz);
    form->addRow("dz:", dzSpinBox);

    subticksSpinBox = new QSpinBox(this);
    subticksSpinBox->setRange(0, 100);
	subticksSpinBox->setValue(docker->subticks);
    form->addRow("subticks:", subticksSpinBox);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(buttons, &QDialogButtonBox::accepted, this, [=](){
			docker->dx = dxSpinBox->value();
			docker->dt = dtSpinBox->value();
			docker->dz = dzSpinBox->value();
			docker->subticks = subticksSpinBox->value();
			docker->replot();
			});
}
