#include "tabbedworkspacewidget.h"
#include "mainwindow.h"


TabbedWorkspaceWidget::TabbedWorkspaceWidget(QString name, QMainWindow* mainWin, QMainWindow* parent): name(name), mainWindow(mainWin), QWidget(parent) 
{  
	tabWidget = new QTabWidget(parent);
	tabWidget->setTabsClosable(true);
	mainWindow->setCentralWidget(tabWidget);
	connect(tabWidget, &QTabWidget::tabCloseRequested, this, &TabbedWorkspaceWidget::on_tabClose);
}

TabbedWorkspaceWidget::~TabbedWorkspaceWidget()
{
}

void TabbedWorkspaceWidget::on_tabClose(int index)
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->widget(index));
	if(!docker)
		return;
	docker->deleteLater();
	tabWidget->removeTab(index);
}

void TabbedWorkspaceWidget::addTab(std::shared_ptr<Profile> profile)
{
	auto path = tr(profile->path.c_str());
	auto docker = new ProfileDocker(path);
	auto widget = docker->addRadargramView(profile, path);
	if(!widget)
		return;

	docker->processingSteps[path] = std::move(profile);
	tabWidget->addTab(docker, path);
	tabWidget->setCurrentWidget(docker);
}
