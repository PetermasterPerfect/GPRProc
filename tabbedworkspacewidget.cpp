#include "tabbedworkspacewidget.h"
#include "mainwindow.h"

std::map<QString, TabbedWorkspaceWidget*> TabbedWorkspaceWidget::instances;

TabbedWorkspaceWidget::TabbedWorkspaceWidget(QString name, QMainWindow* mainWin, QMainWindow* parent): name(name), mainWindow(mainWin), QWidget(parent) 
{  
	//if (TabbedWorkspaceWidget::instances.count(name) > 0)
    //	throw std::runtime_error("This is not supposed to happen");
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
	auto docker = dynamic_cast<ads::CDockManager*>(tabWidget->widget(index));
	if(!docker)
		return;
	
	tabWidget->removeTab(index);
	//docker->deleteLater();
}

ads::CDockManager* TabbedWorkspaceWidget::addTab(Profile profile)
{
	auto docker = new ads::CDockManager(tabWidget);
	auto widget = docker->createDockWidget("plot"+QUuid::createUuid().toString());
	widget->setFeatures(widget->features() & ~ads::CDockWidget::DockWidgetClosable);
	auto plot = profile.createGraph();
	if(!plot)
		return nullptr;
	widget->setWidget(plot);
	docker->addDockWidget(ads::TopDockWidgetArea, widget);

	tabWidget->addTab(docker, "dsads");
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
