
#include "proceduresdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "DockManager.h"

ProceduresDialog::ProceduresDialog(QTabWidget *tab, QWidget *parent)
    : tabWidget(tab), QDialog(parent)
{
    setWindowTitle("Procedures");
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if (!docker)
		return;

    QVBoxLayout *radioLayout = new QVBoxLayout(this);
	for(int i=0; i<proceduresRadios.size(); i++)
	{
		proceduresRadios[i] = new QRadioButton(gProceduresNames[i], this);
		radioLayout->addWidget(proceduresRadios[i]);
	}
	proceduresRadios[0]->setChecked(true); //dc-shift radio button
	
    QWidget *radioWidget = new QWidget(this);
    radioWidget->setLayout(radioLayout);

	procStepsCombo = new MyQComboBox(this);
	int i=0;
	for(auto &it : docker->processingSteps)
		procStepsCombo->insertItem(i++, it.first);
	
	setupStackedOptions();

	auto lineA = new QFrame(this);
	lineA->setFrameShape(QFrame::HLine);
	lineA->setFrameShadow(QFrame::Sunken);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
mainLayout->addWidget(radioWidget);
    mainLayout->addWidget(procStepsCombo);
	mainLayout->addWidget(lineA);
    mainLayout->addWidget(stack);

	procedur = &Profile::subtractDcShift;

	connect(procStepsCombo, &MyQComboBox::signalPopupShown, this, &ProceduresDialog::onPopupUpdate);
	for (int i = 0; i < proceduresRadios.size(); i++) 
		connect(proceduresRadios[i], &QRadioButton::toggled, this, onProcSlots[i]);

	connect(tabWidget, &QTabWidget::currentChanged, this, [=]() {
				onPopupUpdate();
				while(stack->count())
					stack->removeWidget(stack->currentWidget());
				setupStackedOptions();
				mainLayout->removeWidget(stack);
				mainLayout->addWidget(stack);

				for(int i=0; i<proceduresRadios.size(); i++)
					if(proceduresRadios[i]->isChecked())
					{
						stack->setCurrentIndex(i);
						break;
					}
			});
}

void ProceduresDialog::setupStackedOptions()
{
    // procedures pages
    auto dcShiftPage = createDcshift();
	auto dewowPage = createDewow();
	auto gainPage = createGain();
	auto amplitudesTo0Page = createAmplitudesto0();
	auto xFlipPage = createXflip();
	auto yFlipPage = createYflip();
	auto timeCutPage = createTimecut();
	auto moveStartTimePage = createMovestarttime();
	auto butterworthFilterPage = createButterworthfilter();
	auto agcPage = createAgc();
	auto backgroundRemovalPage = createBackgroundremoval();
	auto horizontalScalePage = createHorizontalscale();
    stack = new QStackedWidget;
    stack->addWidget(dcShiftPage);
	stack->addWidget(dewowPage);
	stack->addWidget(gainPage);
	stack->addWidget(amplitudesTo0Page);
	stack->addWidget(xFlipPage);
	stack->addWidget(yFlipPage);
	stack->addWidget(timeCutPage);
	stack->addWidget(moveStartTimePage);
	stack->addWidget(butterworthFilterPage);
	stack->addWidget(agcPage);
	stack->addWidget(backgroundRemovalPage);
	stack->addWidget(horizontalScalePage);
}

std::shared_ptr<Profile> ProceduresDialog::getCurrentProcessing()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	return docker->processingSteps[procStepsCombo->currentText()];
}

void ProceduresDialog::onPopupUpdate()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return;
	for(int i=procStepsCombo->count(); i>=0; i--)
		procStepsCombo->removeItem(i);

	int i=0;
	for(auto &it : docker->processingSteps)
		procStepsCombo->insertItem(i++, it.first);
}

void ProceduresDialog::apply(ProfileDocker *docker, std::shared_ptr<Profile> profile, QString name)
{
	if(docker->anonymousProc.second)
		for(auto widget : docker->dockWidgets())
			if(docker->anonymousProc.first == widget->objectName())
            {
				docker->removeDockWidget(widget);
				break;
			}
	applyBase(docker, profile, name);

	docker->anonymousProc.second = profile;
	docker->anonymousProc.first = name;
}

void ProceduresDialog::applyProc(ProfileDocker *docker, std::shared_ptr<Profile> profile, QString name)
{
	applyBase(docker, profile, name);

	docker->anonymousProc = std::make_pair("", nullptr);
	docker->processingSteps[name] = profile;
}

void ProceduresDialog::applyBase(ProfileDocker *docker, std::shared_ptr<Profile> profile, QString name)
{
	auto widget = docker->addRadargramView(profile, name);
    if(!widget)
        return;
	docker->addDockWidget(ads::BottomDockWidgetArea, widget);
}


void ProceduresDialog::addProcessing(ProfileDocker *docker, QPushButton *procButton)
{
	if(procButton->isFlat())
		return;
	if(docker->anonymousProc.second)
	{
		docker->processingSteps.insert(docker->anonymousProc);
		docker->anonymousProc = std::make_pair("", nullptr);
	}
	procButton->setFlat(true);
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


void ProceduresDialog::onDcshift(bool checked) 
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


void ProceduresDialog::onDewow(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(1);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::subtractDewow;
	}
}


void ProceduresDialog::onGain(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(2);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::gainFunction;
	}
}


void ProceduresDialog::onAmplitudesto0(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(3);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::ampltitudesTo0;
	}
}


void ProceduresDialog::onXflip(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(4);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::xFlip;
	}
}


void ProceduresDialog::onYflip(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(5);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::yFlip;
	}
}


void ProceduresDialog::onTimecut(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(6);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::timeCut;
	}
}


void ProceduresDialog::onMovestarttime(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(7);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::moveStartTime;
	}
}


void ProceduresDialog::onButterworthfilter(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(8);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::butterworthFilter;
	}
}


void ProceduresDialog::onAgc(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(9);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::agc;
	}
}


void ProceduresDialog::onBackgroundremoval(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(10);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::backgroundRemoval;
	}
}


void ProceduresDialog::onHorizontalscale(bool checked) 
{
    if (checked)
    {
        stack->setCurrentIndex(11);
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::horizontalScale;
	}
}

QWidget* ProceduresDialog::createDcshift()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Subtract DC-shift"));
    page->setLayout(layout);
    
    auto input0 = new QDoubleSpinBox;
input0->setRange(0, profile->timeWindow);
input0->setValue(0);
input0->setSingleStep(0.001);
input0->setDecimals(3);
	
auto input1 = new QDoubleSpinBox;
input1->setRange(0, profile->timeWindow);
input1->setValue(profile->timeWindow);
input1->setSingleStep(0.001);
input1->setDecimals(3);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Time window 1: "));
    hLayout1_0->addWidget(input0);
    layout->addLayout(hLayout1_0);
auto hLayout1_1 = new QHBoxLayout;
    hLayout1_1->addWidget(new QLabel("Time window 2: "));
    hLayout1_1->addWidget(input1);
    layout->addLayout(hLayout1_1);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input0->value(), input1->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input0->value(), input1->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createDewow()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Subtract mean (dewow)"));
    page->setLayout(layout);
    
    auto input2 = new QDoubleSpinBox;
input2->setRange(0, profile->timeWindow);
input2->setValue(0);
input2->setSingleStep(0.001);
input2->setDecimals(3);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Time window 1: "));
    hLayout1_0->addWidget(input2);
    layout->addLayout(hLayout1_0);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input2->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input2->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createGain()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Exponent gain"));
    page->setLayout(layout);
    
    auto input3 = new QDoubleSpinBox;
input3->setRange(0, profile->timeWindow);
input3->setValue(0);
input3->setSingleStep(0.001);
input3->setDecimals(3);
	
auto input4 = new QDoubleSpinBox;
input4->setRange(0, profile->timeWindow);
input4->setValue(profile->timeWindow);
input4->setSingleStep(0.001);
input4->setDecimals(3);
	
auto input5 = new QDoubleSpinBox;
input5->setValue(0);
input5->setMinimum(0);
input5->setSingleStep(0.001);
input5->setDecimals(3);
	
auto input6 = new QSpinBox;
input6->setRange(0, 100000000);
input6->setValue(100000);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Start time: "));
    hLayout1_0->addWidget(input3);
    layout->addLayout(hLayout1_0);
auto hLayout1_1 = new QHBoxLayout;
    hLayout1_1->addWidget(new QLabel("End time: "));
    hLayout1_1->addWidget(input4);
    layout->addLayout(hLayout1_1);
auto hLayout1_2 = new QHBoxLayout;
    hLayout1_2->addWidget(new QLabel("Exponent: "));
    hLayout1_2->addWidget(input5);
    layout->addLayout(hLayout1_2);
auto hLayout1_3 = new QHBoxLayout;
    hLayout1_3->addWidget(new QLabel("Max value: "));
    hLayout1_3->addWidget(input6);
    layout->addLayout(hLayout1_3);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float, float, float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input3->value(), input4->value(), input5->value(), input6->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float, float, float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input3->value(), input4->value(), input5->value(), input6->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createAmplitudesto0()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Amplitudes to 0"));
    page->setLayout(layout);
    
        float maxVal = profile->maxAmplitude();
        float minVal = profile->minAmplitude();
        float range = sqrt(maxVal*maxVal-minVal*minVal);
    
    auto input7 = new QDoubleSpinBox;
input7->setRange(minVal, maxVal);
input7->setValue(minVal);
input7->setSingleStep(range/1000);
input7->setDecimals(3);
	
auto input8 = new QDoubleSpinBox;
input8->setRange(minVal, maxVal);
input8->setValue(maxVal);
input8->setSingleStep(range/1000);
input8->setDecimals(3);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Min amplitude: "));
    hLayout1_0->addWidget(input7);
    layout->addLayout(hLayout1_0);
auto hLayout1_1 = new QHBoxLayout;
    hLayout1_1->addWidget(new QLabel("Max amplitude: "));
    hLayout1_1->addWidget(input8);
    layout->addLayout(hLayout1_1);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float )>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input7->value(), input8->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float )>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input7->value(), input8->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createXflip()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("X(traces) flip"));
    page->setLayout(layout);
    
    //inputs
    

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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)()>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)();
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)()>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)();
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createYflip()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Y(samples) flip"));
    page->setLayout(layout);
    
    //inputs
    

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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)()>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)();
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)()>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)();
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createTimecut()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Time cut"));
    page->setLayout(layout);
    
    auto input9 = new QDoubleSpinBox;
input9->setRange(0, profile->timeWindow*10);
input9->setValue(profile->timeWindow);
input9->setSingleStep(profile->timeWindow/profile->samples);
input9->setDecimals(3);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Time to cut: "));
    hLayout1_0->addWidget(input9);
    layout->addLayout(hLayout1_0);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input9->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input9->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createMovestarttime()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Move start time"));
    page->setLayout(layout);
    
    auto input10 = new QDoubleSpinBox;
input10->setRange(-1*profile->timeWindow+profile->timeWindow/profile->samples, profile->timeWindow-profile->timeWindow/profile->samples);
input10->setValue(0);
input10->setSingleStep(profile->timeWindow/profile->samples);
input10->setDecimals(3);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Start time: "));
    hLayout1_0->addWidget(input10);
    layout->addLayout(hLayout1_0);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input10->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input10->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createButterworthfilter()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Butterworth filter"));
    page->setLayout(layout);
    
    auto input11 = new QDoubleSpinBox;
input11->setRange(0, profile->fs()/2-1);
input11->setValue(0);
input11->setSingleStep(1);
input11->setDecimals(3);
	
auto input12 = new QDoubleSpinBox;
input12->setRange(0, profile->fs()/2-1);
input12->setValue(profile->fs()/1e+6/2-1);
input12->setSingleStep(1);
input12->setDecimals(3);
	
auto input13 = new QDoubleSpinBox;
input13->setRange(1, 1000);
input13->setValue(1);
input13->setSingleStep(0.1);
	
auto input14 = new QDoubleSpinBox;
input14->setRange(1, 1000);
input14->setValue(1);
input14->setSingleStep(0.1);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Low cutoff (MHz): "));
    hLayout1_0->addWidget(input11);
    layout->addLayout(hLayout1_0);
auto hLayout1_1 = new QHBoxLayout;
    hLayout1_1->addWidget(new QLabel("High cutoff (MHz): "));
    hLayout1_1->addWidget(input12);
    layout->addLayout(hLayout1_1);
auto hLayout1_2 = new QHBoxLayout;
    hLayout1_2->addWidget(new QLabel("Stopband attenuation [dB]: "));
    hLayout1_2->addWidget(input13);
    layout->addLayout(hLayout1_2);
auto hLayout1_3 = new QHBoxLayout;
    hLayout1_3->addWidget(new QLabel("Passband ripple [dB]: "));
    hLayout1_3->addWidget(input14);
    layout->addLayout(hLayout1_3);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float, float, float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input11->value(), input12->value(), input13->value(), input14->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float, float, float)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input11->value(), input12->value(), input13->value(), input14->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createAgc()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Automatic gain control (AGC)"));
    page->setLayout(layout);
    
    auto input15 = new QSpinBox;
input15->setRange(1, profile->samples);
input15->setValue(1);
input15->setSingleStep(1);
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Window: "));
    hLayout1_0->addWidget(input15);
    layout->addLayout(hLayout1_0);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(size_t)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input15->value());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(size_t)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input15->value());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createBackgroundremoval()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Background removal"));
    page->setLayout(layout);
    
    auto input16 = new QDoubleSpinBox;
input16->setRange(0, profile->timeWindow-profile->timeWindow/profile->samples);
input16->setValue(0);
input16->setSingleStep(0.001);
input16->setDecimals(3);
	
auto input17 = new QDoubleSpinBox;
input17->setRange(0, profile->timeWindow-profile->timeWindow/profile->samples);
input17->setValue(profile->timeWindow-profile->timeWindow/profile->samples);
input17->setSingleStep(0.001);
input17->setDecimals(3);
	
auto input18 = new QDoubleSpinBox;
input18->setRange(0, profile->traces-1);
input18->setValue(0);
input18->setSingleStep(1);
	
auto input19 = new QDoubleSpinBox;
input19->setRange(0, profile->traces-1);
input19->setValue(profile->traces-1);
input19->setSingleStep(1);
	
auto input20 = new QComboBox;
input20->insertItem(0, "Whole trace");
input20->insertItem(1, "Part trace");
input20->insertItem(2, "Mean inside");
input20->insertItem(3, "All inside");
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("Start time: "));
    hLayout1_0->addWidget(input16);
    layout->addLayout(hLayout1_0);
auto hLayout1_1 = new QHBoxLayout;
    hLayout1_1->addWidget(new QLabel("End time: "));
    hLayout1_1->addWidget(input17);
    layout->addLayout(hLayout1_1);
auto hLayout1_2 = new QHBoxLayout;
    hLayout1_2->addWidget(new QLabel("Start trace: "));
    hLayout1_2->addWidget(input18);
    layout->addLayout(hLayout1_2);
auto hLayout1_3 = new QHBoxLayout;
    hLayout1_3->addWidget(new QLabel("End trace: "));
    hLayout1_3->addWidget(input19);
    layout->addLayout(hLayout1_3);
auto hLayout1_4 = new QHBoxLayout;
    hLayout1_4->addWidget(new QLabel("Type: "));
    hLayout1_4->addWidget(input20);
    layout->addLayout(hLayout1_4);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float, size_t, size_t, char )>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input16->value(), input17->value(), input18->value(), input19->value(), input20->currentIndex());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(float, float, size_t, size_t, char )>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input16->value(), input17->value(), input18->value(), input19->value(), input20->currentIndex());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}


QWidget* ProceduresDialog::createHorizontalscale()
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("Horizontal scale"));
    page->setLayout(layout);
    
    auto input21 = new QSpinBox;
input21->setRange(1, profile->traces-1);
input21->setValue(1);
input21->setSingleStep(1);
	
auto input22 = new QComboBox;
input22->insertItem(0, "Stack");
input22->insertItem(1, "Skip");
//inputs
    auto hLayout1_0 = new QHBoxLayout;
    hLayout1_0->addWidget(new QLabel("End trace: "));
    hLayout1_0->addWidget(input21);
    layout->addLayout(hLayout1_0);
auto hLayout1_1 = new QHBoxLayout;
    hLayout1_1->addWidget(new QLabel("Type: "));
    hLayout1_1->addWidget(input22);
    layout->addLayout(hLayout1_1);


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

    connect(applyButton, &QPushButton::clicked, this, [=](){
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(int, char)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input21->value(), input22->currentIndex());
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            });

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)(int, char)>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)(input21->value(), input22->currentIndex());
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        });

    connect(procButton, &QPushButton::clicked, this, [=]() {
            addProcessing(docker, procButton);
        });

    return page;

}

