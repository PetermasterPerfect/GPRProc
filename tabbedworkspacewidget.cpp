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
	auto widget = docker->createDockWidget(path);
	widget->setFeatures(widget->features() & 
			~ads::CDockWidget::DockWidgetClosable & ~ads::CDockWidget::DockWidgetFloatable);
	auto plotPair = profile->createRadargram();
	if(!plotPair)
		return nullptr;
	widget->setWidget(plotPair.value().first);
	docker->radargram2ColorMap.insert(plotPair.value());
	docker->addDockWidget(ads::TopDockWidgetArea, widget);
	docker->processingSteps[path] = std::move(profile);
	tabWidget->addTab(docker, path);
	tabWidget->setCurrentWidget(docker);

	connect(widget, &ads::CDockWidget::closed, docker, [=]() {
				std::cout << "close plot1\n";
				docker->removeDockWidget(widget);
				docker->removeColorMap(plotPair.value().first);
			});
	return docker;
}

TabbedWorkspaceWidget* TabbedWorkspaceWidget::getInstance(const QString& key)
{
	auto ret = instances.find(key);
	if(ret != instances.end())
		return ret->second;
	return nullptr;
}
