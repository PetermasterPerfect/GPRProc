#include <QtWidgets>

#include "mainwindow.h"
#include "profile.h"

//! [0]
MainWindow::MainWindow(char *fname)
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);
//! [0]

//! [1]
    QWidget *topFiller = new QWidget;
	//topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	customPlot = new QCustomPlot();
    customPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	createGraph(fname);

	QWidget *bottomFiller = new QWidget;
    //bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(topFiller);
    layout->addWidget(customPlot);
    layout->addWidget(bottomFiller);
    widget->setLayout(layout);
//! [1]

//! [2]
    createActions();
    createMenus();

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);

    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(480, 320);
}


void MainWindow::createGraph(char *fname)
{
	Profile profile(fname);

	// create graph and assign data to it:
	customPlot->addGraph();
	//customPlot->graph(0)->setData(x, y);
	// give the axes some labels:
	customPlot->xAxis->setLabel("x");
	customPlot->yAxis->setLabel("y");
	customPlot->yAxis->setRangeReversed(true);
	QCPRange *samplesRange = new QCPRange(1, profile.samples);
	QCPRange *tracesRange = new QCPRange(1, profile.lastTrace);
	QCPColorMapData *mapData = new QCPColorMapData(profile.lastTrace, profile.samples, *tracesRange, *samplesRange);
	for(size_t i=1; i<=profile.lastTrace; i++)
		for(size_t j=1; j<=profile.samples; j++)
			mapData->setData(i, j, profile.data[(i-1)*profile.samples+(j-1)]);
	QCPColorMap *map = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
	map->setData(mapData);
	map->rescaleDataRange();
	// set axes ranges, so we see all data:
	customPlot->resize(500, 500);
	customPlot->rescaleAxes();
	customPlot->replot();
}
//! [2]

//! [3]
#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU
	   
void MainWindow::open()
{
    QMessageBox::about(this, tr("OPEN"),
            tr("The <b>Menu</b> example shows how to create "
               "menu-bar menus and context menus."));
}
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

void MainWindow::createActions()
{

    openAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
                          tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);


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
}
