#include <QtWidgets>
#include "mainwindow.h"
#include <iostream>

MainWindow::MainWindow(char *fname)
{
	mainTab = new TabbedWorkspaceWidget("main", this, this);

	//connect(tabWidget, &QTabWidget::tabCloseRequested,
    //    this, &MainWindow::closeTab);
    createActions();
    createMenus();

    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(480, 320);
	
	connect(mainTab->tabWidget, &QTabWidget::tabCloseRequested, this, [=]() {
			wiggleViewAct->setChecked(false);
			});

	connect(mainTab->tabWidget, &QTabWidget::currentChanged, this, [=]() {
			std::cout << "Change\n";
			});
}

#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU
	   

void MainWindow::print()
{
    QMessageBox::about(this, tr("PRINT"),
            tr("The <b>Menu</b> example shows how to create "
               "menu-bar menus and context menus."));
}

void MainWindow::undo()
{
    QMessageBox::about(this, tr("UNDO"),
            tr("The <b>Menu</b> example shows how to create "
               "menu-bar menus and context menus."));
}

void MainWindow::redo()
{
    QMessageBox::about(this, tr("REDO"),
            tr("The <b>Menu</b> example shows how to create "
               "menu-bar menus and context menus."));
}

void MainWindow::wiggleView()
{
	auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
	if(!docker)
		return;
	if(docker->isWiggled())
	{
		removeWiggle(docker);
		return;
	}

	setUpWiggle(docker, 1);
}

void MainWindow::removeWiggle(ProfileDocker *docker)
{
	auto widget = docker->findDockWidget("Wiggle view");
	if(widget)
		docker->removeDockWidget(widget);
	docker->setWiggled(false);
	wiggleViewAct->setChecked(false);
}


void MainWindow::setUpWiggle(ProfileDocker *docker, size_t n)
{
	auto container = new QWidget();
	auto optionsContainer = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout(container);
	QVBoxLayout *optionsLayout = new QVBoxLayout(optionsContainer);
	QSpinBox *spinBox = new QSpinBox();
	QRadioButton *currentButton = new QRadioButton("current");
	QRadioButton *amplitudeButton = new QRadioButton("amplitude");
	QRadioButton *phaseButton = new QRadioButton("phase");
	auto widget = docker->createDockWidget("Wiggle view");
	auto wiggle = docker->profile.createWiggle(n);

	spinBox->setFixedWidth(100);
	spinBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
	spinBox->setRange(1, docker->profile.lastTrace);
	spinBox->setValue(n);
	wiggle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	layout->addWidget(optionsContainer, 0, Qt::AlignLeft | Qt::AlignTop);
	layout->addWidget(wiggle);
	optionsLayout->addWidget(spinBox);
	optionsLayout->addWidget(currentButton);
	optionsLayout->addWidget(amplitudeButton);
	optionsLayout->addWidget(phaseButton);
	widget->setWidget(container);
	docker->addDockWidget(ads::TopDockWidgetArea, widget);
	connect(widget, &ads::CDockWidget::closed, docker, [=]() {
			wiggleViewAct->setChecked(false);
			docker->setWiggled(false);
			});
	connect(spinBox, &QSpinBox::valueChanged, this, [=](int i) {
			removeWiggle(docker);
			setUpWiggle(docker, i);
			});
	docker->setWiggled(true);
}


void MainWindow::createActions()
{
	//open_shortcut.setContext(Qt::ApplicationShortcut);
	//connect(&open_shortcut, &QShortcut::activated, this, &MainWindow::onOpenFile);
    openAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
                          tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);


    printAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrint),
                           tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the document"));
    connect(printAct, &QAction::triggered, this, &MainWindow::print);

    exitAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit),
                          tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    undoAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::EditUndo),
                          tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last operation"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    redoAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::EditRedo),
                          tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redo the last operation"));
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    wiggleViewAct = new QAction(tr("&Wiggle view"), this);
    wiggleViewAct->setStatusTip(tr("Wiggle view of trace"));
	wiggleViewAct->setCheckable(true);
    connect(wiggleViewAct, &QAction::triggered, this, &MainWindow::wiggleView);
} 

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    
	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(wiggleViewAct);
    //editMenu->addAction(undoAct);

}

void MainWindow::onOpenFile()
{
	auto filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "/home/zxcv/geofiz");
	//QString filename = "/home/zxcv/geofiz/GPRdata/ExampleDuneProfile/DuneData.DZT";

	if(!filename.length())
		return;

	Profile prof(filename.toStdString());
	mainTab->addTab(prof);
}
