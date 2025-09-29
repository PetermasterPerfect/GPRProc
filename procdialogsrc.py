INIT = """
#include "proceduresdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ProceduresDialog::ProceduresDialog(QTabWidget *tab, QWidget *parent)
    : tabWidget(tab), QDialog(parent)
{{
    setWindowTitle("Procedures");
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if (!docker)
		return;

    QVBoxLayout *radioLayout = new QVBoxLayout;
	for(int i=0; i<proceduresRadios.size(); i++)
	{{
		proceduresRadios[i] = new QRadioButton(gProceduresNames[i]);
		radioLayout->addWidget(proceduresRadios[i]);
	}}
	proceduresRadios[0]->setChecked(true); //dc-shift radio button
	
    QWidget *radioWidget = new QWidget;
    radioWidget->setLayout(radioLayout);

	procStepsCombo = new MyQComboBox;
	int i=0;
	for(auto &it : docker->processingSteps)
		procStepsCombo->insertItem(i++, it.first);
	
	setupStackedOptions();

	auto lineA = new QFrame;
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

	connect(tabWidget, &QTabWidget::currentChanged, this, [=]() {{
				onPopupUpdate();
				while(stack->count())
					stack->removeWidget(stack->currentWidget());
				setupStackedOptions();
				mainLayout->removeWidget(stack);
				mainLayout->addWidget(stack);

				for(int i=0; i<proceduresRadios.size(); i++)
					if(proceduresRadios[i]->isChecked())
					{{
						stack->setCurrentIndex(i);
						break;
					}}
			}});
}}

void ProceduresDialog::setupStackedOptions()
{{
    // procedures pages
    {}
    stack = new QStackedWidget;
    {}
}}

std::shared_ptr<Profile> ProceduresDialog::getCurrentProcessing()
{{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	return docker->processingSteps[procStepsCombo->currentText()];
}}

void ProceduresDialog::onPopupUpdate()
{{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return;
	for(int i=procStepsCombo->count(); i>=0; i--)
		procStepsCombo->removeItem(i);

	int i=0;
	for(auto &it : docker->processingSteps)
		procStepsCombo->insertItem(i++, it.first);
}}

void ProceduresDialog::apply(ProfileDocker *docker, std::shared_ptr<Profile> profile, QString name)
{{
	if(docker->anonymousProc.second)
		for(auto widget : docker->dockWidgets())
			if(docker->anonymousProc.first == widget->objectName())
            {{
				docker->removeDockWidget(widget);
				break;
			}}
	applyBase(docker, profile, name);

	docker->anonymousProc.second = profile;
	docker->anonymousProc.first = name;
}}

void ProceduresDialog::applyProc(ProfileDocker *docker, std::shared_ptr<Profile> profile, QString name)
{{
	applyBase(docker, profile, name);

	docker->anonymousProc = std::make_pair("", nullptr);
	docker->processingSteps[name] = profile;
}}

void ProceduresDialog::applyBase(ProfileDocker *docker, std::shared_ptr<Profile> profile, QString name)
{{
	auto widget = docker->createDockWidget(name);
	auto plotPair = profile->createRadargram(docker->gradType, docker->scale);
	if(!plotPair.value().first)
		return;

	connect(widget, &ads::CDockWidget::closed, docker, [=]() {{
			docker->removeDockWidget(widget);
			docker->removeColorMap(plotPair.value().first);
		}});
	widget->setWidget(plotPair.value().first);
	docker->radargram2ColorMap.insert(plotPair.value());
	docker->addDockWidget(ads::BottomDockWidgetArea, widget);
}}


void ProceduresDialog::addProcessing(ProfileDocker *docker, QPushButton *procButton)
{{
	if(procButton->isFlat())
		return;
	if(docker->anonymousProc.second)
	{{
		docker->processingSteps.insert(docker->anonymousProc);
		docker->anonymousProc = std::make_pair("", nullptr);
	}}
	procButton->setFlat(true);
}}

QString ProceduresDialog::getProcessingName(ProfileDocker* docker, QLineEdit *procName)
{{
	auto txt = procName->text();
	if(!txt.size())
		for(auto &radio : proceduresRadios)
			if(radio->isChecked())
				return radio->text()+QString::number(docker->processingSteps.size());
	return txt;

}}

"""


ON_PROCEDUR = """
void ProceduresDialog::{}(bool checked) 
{{
    if (checked)
    {{
        stack->setCurrentIndex({});
		auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
		if(!docker)
			return;
		procedur = &Profile::{};
	}}
}}

"""

CREATE_PAGE = """QWidget* ProceduresDialog::{}()
{{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if(!docker)
		return nullptr;
	auto page = new QWidget;
	auto layout = new QVBoxLayout;
	auto profile = getCurrentProcessing();

    layout->addWidget(new QLabel("{}"));
    page->setLayout(layout);
    {extra}
    {}//inputs
    {}

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

    connect(applyButton, &QPushButton::clicked, this, [=](){{
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)({})>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)({});
            if(!proccessedProf)
                return;
            apply(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(false);
            }});

    connect(applyProcButton, &QPushButton::clicked, this, [=]() {{
            auto procFunc = std::any_cast<std::shared_ptr<Profile> (Profile::*)({})>(procedur);
            auto profile = getCurrentProcessing();
            auto proccessedProf = (profile.get()->*procFunc)({});
            if(!proccessedProf)
                return;
            applyProc(docker, proccessedProf, getProcessingName(docker, procName));
            procButton->setFlat(true);

        }});

    connect(procButton, &QPushButton::clicked, this, [=]() {{
            addProcessing(docker, procButton);
        }});

    return page;

}}

"""
