#include "tabbedworkspacewidget.h"
#include "mainwindow.h"


TabbedWorkspaceWidget::TabbedWorkspaceWidget(QString name, QMainWindow* mainWin, QMainWindow* parent): name(name), mainWindow(mainWin), QWidget(parent) 
{  
	tabWidget = new QTabWidget(parent);
	tabWidget->setTabsClosable(true);
	//tabWidget->setMovable(true);
	mainWindow->setCentralWidget(tabWidget);
	addTab(std::make_shared<Profile>());
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
	
	tabWidget->removeTab(index);
}

ProfileDocker* TabbedWorkspaceWidget::addTab(std::shared_ptr<Profile> profile)
{
	auto path = tr(profile->path.c_str());
	auto docker = new ProfileDocker(path, tabWidget);
	auto widget = docker->addRadargramView(profile, path, tabWidget);
	if(!widget)
		return nullptr;

	docker->processingSteps[path] = std::move(profile);
	tabWidget->addTab(docker, path);
	tabWidget->setCurrentWidget(docker);
	return docker;
}
