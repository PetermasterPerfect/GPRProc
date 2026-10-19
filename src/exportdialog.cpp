#include "exportdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QPushButton>

ExportDialog::ExportDialog(QTabWidget *tab, QWidget *parent)
    : QDialog(parent), tabWidget(tab)
{
    setWindowTitle("Export");
    auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
    if (!docker)
        return;

    auto procStepsCombo = new MyQComboBox(this);
    int i = 0;
    for (auto &it : docker->processingSteps)
        procStepsCombo->insertItem(i++, it.first);

    auto formatCombo = new QComboBox(this);
    formatCombo->addItems({"pdf", "jpg", "png", "bmp"});

    auto *form = new QFormLayout;
    form->addRow("Processing step:", procStepsCombo);
    form->addRow("Format:", formatCombo);

	auto widthSpinBox = new QSpinBox(this);
	widthSpinBox->setRange(0, 20000);
	widthSpinBox->setValue(1200);
	widthSpinBox->setSuffix(" px");
	widthSpinBox->setSpecialValueText("auto");

	auto heightSpinBox = new QSpinBox(this);
	heightSpinBox->setRange(0, 20000);
	heightSpinBox->setValue(900);
	heightSpinBox->setSuffix(" px");
	heightSpinBox->setSpecialValueText("auto");

	form->addRow("Width:", widthSpinBox);
	form->addRow("Height:", heightSpinBox);

    auto *buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Cancel);
    QPushButton *exportBtn = buttons->addButton("Export", QDialogButtonBox::AcceptRole);
    exportBtn->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, [=](){
		bool ret;
		auto profile = docker->processingSteps[procStepsCombo->currentText()];
		const QString fmt = formatCombo->currentText();
		QString path = QFileDialog::getSaveFileName(this, tr("Save %1").arg(fmt),
													QDir::currentPath(),
													tr("%1 file (*.%1)").arg(fmt));
		if(path.isEmpty())
			return;

		if(!path.endsWith("." + fmt, Qt::CaseInsensitive))
			path += "." + fmt;

		auto opt =  docker->singlePlot(this, profile);
		if(!opt)
		{
			QMessageBox::critical(this, tr("Export"),
            	tr("Failed to save the file:\n%1").arg(path));
			return;
		}

		auto radargram = opt.value();
		docker->extendPlot(profile, radargram.first, radargram.second);
		auto plot = radargram.first;
		auto width = widthSpinBox->value();
		auto height = heightSpinBox->value();

		if(fmt == "pdf")      ret = plot->savePdf(path, width, height);
		else if(fmt == "jpg") ret = plot->saveJpg(path, width, height);
		else if(fmt == "png") ret = plot->savePng(path, width, height, 2.0, -1, 150);
		else if(fmt == "bmp") ret = plot->saveBmp(path, width, height, 2.0, 150);

		if(ret)
			QMessageBox::information(this, tr("Export"),
				tr("Saved successfully to:\n%1").arg(path));
		else
			QMessageBox::critical(this, tr("Export"),
            	tr("Failed to save the file:\n%1").arg(path));

		});
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(buttons);
}
