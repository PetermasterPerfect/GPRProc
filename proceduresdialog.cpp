#include "proceduresdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ProceduresDialog::ProceduresDialog(QTabWidget *tab, QWidget *parent)
    : tabWidget(tab), QDialog(parent)
{
    setWindowTitle("Procedures");
    QVBoxLayout *radioLayout = new QVBoxLayout;
	for(int i=0; i<proceduresRadios.size(); i++)
	{
		proceduresRadios[i] = new QRadioButton(proceduresNames[i]);
		radioLayout->addWidget(proceduresRadios[i]);
	}
	proceduresRadios[0]->setChecked(true); //dc shift
	
    QWidget *radioWidget = new QWidget;
    radioWidget->setLayout(radioLayout);



	setupStackedOptions();

	auto lineA = new QFrame;
	lineA->setFrameShape(QFrame::HLine);
	lineA->setFrameShadow(QFrame::Sunken);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(radioWidget);
	mainLayout->addWidget(lineA);
    mainLayout->addWidget(stack);

	procedur = &Profile::subtractDcShift;

	    // Connect signals
    connect(proceduresRadios[0], &QRadioButton::toggled, this, &ProceduresDialog::onDcShift);
    connect(proceduresRadios[1], &QRadioButton::toggled, this, &ProceduresDialog::onDewow);
	connect(tabWidget, &QTabWidget::currentChanged, this, [=]() {
				while(stack->count())
					stack->removeWidget(stack->currentWidget());
				setupStackedOptions();
				mainLayout->removeWidget(stack);
				mainLayout->addWidget(stack);

				for(auto &radio : proceduresRadios)
					if(radio->isChecked())
						break;
			});
}

void ProceduresDialog::setupStackedOptions()
{
	QWidget *dcShiftPage = createDcShiftPage();
    //QWidget *dewowPage = createDewowPage();
    stack = new QStackedWidget;
    stack->addWidget(dcShiftPage);
    //stack->addWidget(dewowPage);
}

QWidget* ProceduresDialog::createDcShiftPage()
{
	auto dcShiftPage = new QWidget;
	auto layout = new QVBoxLayout;
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if (!docker)
		return nullptr;

	dcTimeWindow1 = new QDoubleSpinBox;
	dcTimeWindow2 = new QDoubleSpinBox;

	dcTimeWindow1->setRange(0, docker->profile.timeWindow);
	dcTimeWindow1->setSingleStep(0.001);
	dcTimeWindow1->setDecimals(3);
	dcTimeWindow1->setValue(0);

	dcTimeWindow2->setRange(0, docker->profile.timeWindow);
	dcTimeWindow2->setSingleStep(0.001);
	dcTimeWindow2->setDecimals(3);
	dcTimeWindow2->setValue(docker->profile.timeWindow);

	dcShiftPage->setLayout(layout);

	layout->addWidget(new QLabel("Subtract DC-shift options"));

	auto hLayout1 = new QHBoxLayout;
	hLayout1->addWidget(new QLabel("Time window 1:"));
	hLayout1->addWidget(dcTimeWindow1);
	layout->addLayout(hLayout1);

	auto hLayout2 = new QHBoxLayout;
	hLayout2->addWidget(new QLabel("Time window 2:"));
	hLayout2->addWidget(dcTimeWindow2);
	layout->addLayout(hLayout2);

	auto hLayout3 = new QHBoxLayout;
	auto procName = new QLineEdit;
	procName->setPlaceholderText("Input processing name(blank for default)");

	auto procButton = new QPushButton("Proc");
	procButton->setStatusTip("Add processing step");
	procButton->setFlat(true);
	hLayout3->addWidget(procName);

	QPushButton *applyButton = new QPushButton("Apply");
	hLayout3->addWidget(applyButton);

	QPushButton *applyProcButton = new QPushButton("Apply&Proc");
	applyProcButton->setStatusTip("Apply and add to processing steps");
	hLayout3->addWidget(applyProcButton);

	hLayout3->addWidget(procButton);
	layout->addLayout(hLayout3);

	connect(applyButton, &QPushButton::clicked, this, [=](){
			auto procFunc = std::any_cast<double* (Profile::*)(double, double)>(procedur);
			auto buf = (docker->profile.*procFunc)(dcTimeWindow1->value(), dcTimeWindow2->value());
			if(!buf)
				return;
			apply(docker, buf, getProcessingName(docker, procName));
			procButton->setFlat(false);
			});

	connect(applyProcButton, &QPushButton::clicked, this, [=]() {
			auto procFunc = std::any_cast<double* (Profile::*)(double, double)>(procedur);
			auto buf = (docker->profile.*procFunc)(dcTimeWindow1->value(), dcTimeWindow2->value());
			if(!buf)
				return;
			applyProc(docker, buf, getProcessingName(docker, procName));
			procButton->setFlat(true);

		});

	connect(procButton, &QPushButton::clicked, this, [=]() {
			addProcessing(docker, procButton);
		});

	return dcShiftPage;
}


void ProceduresDialog::apply(ProfileDocker *docker, double *buf, QString name)
{
	applyBase(docker, buf, name);

	if(docker->anonymousProc.second)
		fftw_free(docker->anonymousProc.second);
	docker->anonymousProc.second = buf;
	docker->anonymousProc.first = name;
}


void ProceduresDialog::applyProc(ProfileDocker *docker, double *buf, QString name)
{
	applyBase(docker, buf, name);

	if(docker->anonymousProc.second)
	{
		fftw_free(docker->anonymousProc.second);
		docker->anonymousProc = std::make_pair("", nullptr);
	}

	docker->processingSteps[name] = buf;
}

void ProceduresDialog::applyBase(ProfileDocker *docker, double *buf, QString name)
{
	std::cout << "applyBase\n";
	auto widget = docker->createDockWidget(name);
	auto plotPair = docker->profile.createRadargram(buf, docker->gradType, docker->scale);
	if(!plotPair.value().first)
		return;

	connect(widget, &ads::CDockWidget::closed, docker, [=]() {
			std::cout << "close plot2\n";
			docker->removeDockWidget(widget);
			docker->removeColorMap(plotPair.value().first);
		});
	widget->setWidget(plotPair.value().first);
	docker->radargram2ColorMap.insert(plotPair.value());
	docker->addDockWidget(ads::BottomDockWidgetArea, widget);
}


void ProceduresDialog::addProcessing(ProfileDocker *docker, QPushButton *procButton)
{
	std::cout << "addprocessing\n";	
	if(procButton->isFlat())
		return;
	if(docker->anonymousProc.second)
	{
		docker->processingSteps.insert(docker->anonymousProc);
		docker->anonymousProc = std::make_pair("", nullptr);
	}
	procButton->setFlat(true);
}

QWidget* ProceduresDialog::createDewowPage()
{
	auto dewowPage = new QWidget;
	auto layout = new QVBoxLayout;
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;

	dewowTimeWindow1 = new QDoubleSpinBox;
	dewowTimeWindow1->setRange(0, docker->profile.timeWindow);
	dewowTimeWindow1->setValue(docker->profile.timeWindow);
	dewowTimeWindow1->setSingleStep(0.001);
	dewowTimeWindow1->setDecimals(3);
	dewowPage->setLayout(layout);

	layout->addWidget(new QLabel("Subtract-mean (dewow)"));

	auto hLayout1 = new QHBoxLayout;

	hLayout1->addWidget(new QLabel("Time window:"));
	hLayout1->addWidget(dewowTimeWindow1);
	layout->addLayout(hLayout1);

	auto hLayout2 = new QHBoxLayout;
	auto procName = new QLineEdit;
	procName->setPlaceholderText("Input processing name(blank for default)");

	auto procButton = new QPushButton("Proc");
	procButton->setStatusTip("Add processing step");
	procButton->setFlat(true);
	hLayout2->addWidget(procName);

	QPushButton *applyButton = new QPushButton("Apply");
	hLayout2->addWidget(applyButton);

	QPushButton *applyProcButton = new QPushButton("Apply&Proc");
	applyProcButton->setStatusTip("Apply and add to processing steps");
	hLayout2->addWidget(applyProcButton);
	hLayout2->addWidget(procButton);
	layout->addLayout(hLayout2);

	return dewowPage;
}

QString ProceduresDialog::getProcessingName(ProfileDocker* docker, QLineEdit *procName)
{
	auto txt = procName->text();
	if(!txt.size())
		for(auto &radio : proceduresRadios)
			if(radio->isChecked())
				return radio->text()+QString::number(docker->processingSteps.size());
	return txt;

}


void ProceduresDialog::onDcShift(bool checked) 
{
    if (checked)
	{
        stack->setCurrentIndex(0);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::subtractDcShift;

	}
}

void ProceduresDialog::onDewow(bool checked) {
    if (checked)
        stack->setCurrentIndex(1);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::subtractDcShift;
}
