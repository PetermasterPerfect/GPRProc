#include "tabbedworkspacewidget.h"
#include "mainwindow.h"

std::map<QString, TabbedWorkspaceWidget*> TabbedWorkspaceWidget::instances;

TabbedWorkspaceWidget::TabbedWorkspaceWidget(QString name, QMainWindow* mainWin, QMainWindow* parent): name(name), mainWindow(mainWin), QWidget(parent) 
{  
	instances[name] = this;
	tabWidget = new QTabWidget(this);
	tabWidget->setTabsClosable(true);
	//tabWidget->setMovable(true);
	mainWindow->setCentralWidget(tabWidget);
	addTab(Profile());
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
	//docker->deleteLater();
}

ProfileDocker* TabbedWorkspaceWidget::addTab(Profile profile)
{
	auto docker = new ProfileDocker(tr(profile.path.c_str()), profile, tabWidget);
	auto widget = docker->createDockWidget("plot"+QUuid::createUuid().toString());
	widget->setFeatures(widget->features() & ~ads::CDockWidget::DockWidgetClosable);
	auto plot = docker->profile.createGraph();
	if(!plot)
		return nullptr;
	widget->setWidget(plot);
	docker->addDockWidget(ads::TopDockWidgetArea, widget);

	tabWidget->addTab(docker, tr(profile.path.c_str()));
	tabWidget->setCurrentWidget(docker);
	return docker;
}

TabbedWorkspaceWidget* TabbedWorkspaceWidget::getInstance(const QString& key)
{
	auto ret = instances.find(key);
	if(ret != instances.end())
		return ret->second;
	return nullptr;
}
