#include <QtWidgets>
#include "mainwindow.h"

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


void MainWindow::closeTab(QObject* object)
{
	this->setFocus();
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
