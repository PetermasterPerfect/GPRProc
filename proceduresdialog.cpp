#include "proceduresdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ProceduresDialog::ProceduresDialog(QTabWidget *tab, QWidget *parent)
    : tabWidget(tab), QDialog(parent)
{
    setWindowTitle("Procuderes");
    dcShiftRadio = new QRadioButton("Subtract DC-shif");
	dcShiftRadio->setChecked(true);
    dewowRadio = new QRadioButton("Subtract mean (dewow)");


    QVBoxLayout *radioLayout = new QVBoxLayout;
    radioLayout->addWidget(dcShiftRadio);
    radioLayout->addWidget(dewowRadio);

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

    // Connect signals
    connect(dcShiftRadio, &QRadioButton::toggled, this, &ProceduresDialog::onDcShift);
    connect(dewowRadio, &QRadioButton::toggled, this, &ProceduresDialog::onDewow);
	connect(tabWidget, &QTabWidget::currentChanged, this, [=]() {
				while(stack->count())
					stack->removeWidget(stack->currentWidget());
				setupStackedOptions();
				mainLayout->removeWidget(stack);
				mainLayout->addWidget(stack);
				if(dcShiftRadio->isChecked())
					stack->setCurrentIndex(0);
				if(dewowRadio->isChecked())
					stack->setCurrentIndex(1);
			});
}

void ProceduresDialog::setupStackedOptions()
{
	QWidget *dcShiftPage = createDcShiftPage();
    QWidget *dewowPage = createDewowPage();
    stack = new QStackedWidget;
    stack->addWidget(dcShiftPage);
    stack->addWidget(dewowPage);
}

QWidget* ProceduresDialog::createDcShiftPage()
{
	auto dcShiftPage = new QWidget;
	auto layout = new QVBoxLayout;
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if (!docker)
		return nullptr;

	QDoubleSpinBox *timeWindow1 = new QDoubleSpinBox();
	QDoubleSpinBox *timeWindow2 = new QDoubleSpinBox();

	timeWindow1->setRange(0, 0);
	timeWindow1->setSingleStep(0.001);
	timeWindow1->setDecimals(3);
	timeWindow1->setValue(docker->profile.timeWindow);

	timeWindow2->setRange(0, docker->profile.timeWindow);
	timeWindow2->setSingleStep(0.001);
	timeWindow2->setDecimals(3);
	timeWindow2->setValue(docker->profile.timeWindow);

	dcShiftPage->setLayout(layout);

	layout->addWidget(new QLabel("Subtract DC-shift options"));

	auto hLayout1 = new QHBoxLayout;
	hLayout1->addWidget(new QLabel("Time window 1:"));
	hLayout1->addWidget(timeWindow1);
	layout->addLayout(hLayout1);

	auto hLayout2 = new QHBoxLayout;
	hLayout2->addWidget(new QLabel("Time window 2:"));
	hLayout2->addWidget(timeWindow2);
	layout->addLayout(hLayout2);

	QPushButton *applyButton = new QPushButton("Apply");
	layout->addWidget(applyButton);
	connect(applyButton, &QPushButton::clicked, this, [=]() {
			std::cout << "apply\n";	
			auto widget = docker->createDockWidget("Subtract DC-shift");
			auto buf = docker->profile.subtractDcShift(timeWindow1->value(), timeWindow2->value());
			if(!buf)
				return;
			auto radargram = docker->profile.createRadargram(buf);
			docker->profile.data = buf;
			radargram->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			widget->setWidget(radargram);
			docker->addDockWidget(ads::BottomDockWidgetArea, widget);

		});
	return dcShiftPage;
}


QWidget* ProceduresDialog::createDewowPage()
{
	auto dewowPage = new QWidget;
	auto layout = new QVBoxLayout;
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;

	QDoubleSpinBox *timeWindow = new QDoubleSpinBox();

	timeWindow->setRange(0, docker->profile.timeWindow);
	timeWindow->setValue(docker->profile.timeWindow);
	timeWindow->setSingleStep(0.001);
	timeWindow->setDecimals(3);
	dewowPage->setLayout(layout);

	layout->addWidget(new QLabel("Subtract-mean (dewow)"));

	auto hLayout1 = new QHBoxLayout;
	hLayout1->addWidget(new QLabel("Time window:"));
	hLayout1->addWidget(timeWindow);
	layout->addLayout(hLayout1);

	QPushButton *applyButton = new QPushButton("Apply");
	layout->addWidget(applyButton);

	return dewowPage;
}



void ProceduresDialog::onDcShift(bool checked) 
{
    if (checked)
	{
        stack->setCurrentIndex(0);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());

	}
}

void ProceduresDialog::onDewow(bool checked) {
    if (checked)
        stack->setCurrentIndex(1);
}
