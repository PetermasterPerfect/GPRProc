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
	createToolbar();

    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(880, 520);
	
	connect(mainTab->tabWidget, &QTabWidget::tabCloseRequested, this, [=]() {
			wiggleViewAct->setChecked(false);
			traceNormalizationAct->setChecked(false);
			colorScaleAct->setChecked(false);
			userMarksAct->setChecked(false);
			userMarksAct->setEnabled(false);
			});

	connect(mainTab->tabWidget, &QTabWidget::currentChanged, this, [=]() {
			auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
			if(!docker)
				return;
			wiggleViewAct->setChecked(docker->wiggle);
			traceNormalizationAct->setChecked(docker->traceNormalization);
			colorScaleAct->setChecked(docker->colorScale);
			userMarksAct->setChecked(docker->userMarks);
			if(docker->containsMarks())
				userMarksAct->setEnabled(true);
			else
				userMarksAct->setEnabled(false);

			for(auto &color : gradientMap)
				if(color.second == docker->gradType)
				{
					colormapCombo->setCurrentText(color.first);
					break;
				}
			scaleSpinBox->setValue(docker->scale);

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


void MainWindow::traceNormalization()
{
	auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
	if(!docker)
		return;

	docker->replot((bool)(docker->traceNormalization ^ true));
}

void MainWindow::colorScale()
{
	auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
	if(!docker)
		return;

	docker->colorScale ^= true;
	docker->replot();
}

void MainWindow::userMarks()
{
	auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
	if(!docker)
		return;

	docker->replotMarks(docker->userMarks ^ true);
}

void MainWindow::wiggleView()
{
	auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
	if(!docker)
		return;
	if(docker->wiggle)
		removeWiggle(docker);
	else
		setUpWiggle(docker, 1);
}

void MainWindow::removeWiggle(ProfileDocker *docker)
{
	if(docker->wiggle)
	{
		docker->removeDockWidget(docker->wiggle);
		docker->wiggle = nullptr;
	}
	wiggleViewAct->setChecked(false);
}


void MainWindow::setUpWiggle(ProfileDocker *docker, size_t n, int idx)
{
	// TODO: clean up this mess
	auto container = new QWidget();
	auto optionsContainer = new QWidget();
	auto procStepsCombo = new MyQComboBox;
	int i=0;
	for(auto &it : docker->processingSteps)
		procStepsCombo->insertItem(i++, it.first);
	procStepsCombo->setCurrentIndex(idx);
	
	QHBoxLayout *layout = new QHBoxLayout(container);
	QVBoxLayout *optionsLayout = new QVBoxLayout(optionsContainer);
	QSpinBox *spinBox = new QSpinBox();
	QRadioButton *currentButton = new QRadioButton("current");
	QRadioButton *amplitudeButton = new QRadioButton("amplitude");
	QRadioButton *phaseButton = new QRadioButton("phase");
	ads::CDockWidget *widget;
	if(docker->wiggle)
	{
		for(auto &child : docker->wiggle->widget()->children())
		if(auto plot = dynamic_cast<QCustomPlot*>(child))
		{
			widget = docker->wiggle;
			plot->clearItems();

			auto profile = docker->processingSteps[procStepsCombo->currentText()];
			auto wiggleData = profile->prepareWiggleData(n, docker->wiggleType);
			plot->graph(0)->setData(wiggleData.first, wiggleData.second);
			plot->removeGraph(1);
			QCPGraph *marker = plot->addGraph();
			marker->addData(profile->picks[n], profile->data[n*profile->samples+profile->picks[n]]);

			marker->setLineStyle(QCPGraph::lsNone);
			marker->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 10));
			marker->setPen(QPen(Qt::red));
			plot->rescaleAxes();
			plot->replot();
			return;
		}
	}
	else
	{
		widget = docker->createDockWidget("Wiggle view");
		docker->wiggle = widget;
	}
	auto profile = docker->processingSteps[procStepsCombo->currentText()];
	auto wiggle = profile->createWiggle(n, docker->wiggleType);
	if(docker->wiggleType == 0)
		currentButton->setChecked(true);
	else if(docker->wiggleType == 1)
		amplitudeButton->setChecked(true);
	else
		phaseButton->setChecked(true);

	spinBox->setFixedWidth(200);
	spinBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
	spinBox->setRange(1, profile->traces);
	spinBox->setValue(n);
	wiggle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	layout->addWidget(optionsContainer, 0, Qt::AlignLeft | Qt::AlignTop);
	layout->addWidget(wiggle);
	optionsLayout->addWidget(procStepsCombo);
	optionsLayout->addWidget(spinBox);
	optionsLayout->addWidget(currentButton);
	optionsLayout->addWidget(amplitudeButton);
	optionsLayout->addWidget(phaseButton);
	widget->setWidget(container);
	docker->addDockWidget(ads::TopDockWidgetArea, widget);
	connect(widget, &ads::CDockWidget::closed, docker, [=]() {
			wiggleViewAct->setChecked(false);
			docker->wiggle = nullptr;
			});
	connect(spinBox, &QSpinBox::valueChanged, this, [=](int i) {
			//removeWiggle(docker);
			setUpWiggle(docker, i-1, procStepsCombo->currentIndex());
			});

	connect(currentButton, &QRadioButton::toggled, this, [=](bool checked) {
			if(checked) {
				docker->wiggleType = 0;
				removeWiggle(docker);
				setUpWiggle(docker, spinBox->value()-1, procStepsCombo->currentIndex());
			}
			});

	connect(amplitudeButton, &QRadioButton::toggled, this, [=](bool checked) {
			if(checked) {
				docker->wiggleType = 1;
				removeWiggle(docker);
				setUpWiggle(docker, spinBox->value()-1, procStepsCombo->currentIndex());
			}
			});
	connect(phaseButton, &QRadioButton::toggled, this, [=](bool checked) {
			if(checked) {
				docker->wiggleType = 2;
				removeWiggle(docker);
				setUpWiggle(docker, spinBox->value()-1, procStepsCombo->currentIndex());
			}
			});

	connect(procStepsCombo, &MyQComboBox::activated, this, [=](int idx) {
			if(idx == -1)
				return;
			setUpWiggle(docker, spinBox->value()-1, idx);
			});
	connect(procStepsCombo, &MyQComboBox::signalPopupShown, this, [=]() {
		for(int i=procStepsCombo->count(); i>=0; i--)
			procStepsCombo->removeItem(i);

		int i=0;
		for(auto &it : docker->processingSteps)
			procStepsCombo->insertItem(i++, it.first);}
			);
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

    traceNormalizationAct = new QAction(tr("&Trace normalization"), this);
    traceNormalizationAct->setStatusTip(tr("Apply trace normalization"));
	traceNormalizationAct->setCheckable(true);
    connect(traceNormalizationAct, &QAction::triggered, this, &MainWindow::traceNormalization);

    colorScaleAct = new QAction(tr("&Color scale"), this);
    colorScaleAct->setStatusTip(tr("Show color scale"));
	colorScaleAct->setCheckable(true);
    connect(colorScaleAct, &QAction::triggered, this, &MainWindow::colorScale);

	userMarksAct = new QAction(tr("&User marks"), this);
    userMarksAct->setStatusTip(tr("Show user marks"));
	userMarksAct->setCheckable(true);
    connect(userMarksAct, &QAction::triggered, this, &MainWindow::userMarks);

    proceduresAct = new QAction(tr("&Procedures"), this);
    connect(proceduresAct, &QAction::triggered, this, &MainWindow::showpProceduresDialog);

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
	viewMenu->addAction(traceNormalizationAct);
	viewMenu->addAction(colorScaleAct);
	viewMenu->addAction(userMarksAct);

	processingMenu = menuBar()->addMenu(tr("&Processing"));
	processingMenu->addAction(proceduresAct);
}


void MainWindow::createToolbar()
{
	toolBar = addToolBar("Toolbar");
	toolBar->addAction(openAct);
	colormapCombo = new QComboBox;
	for(auto &color : gradientMap)
		colormapCombo->addItem(color.first);
	toolBar->addWidget(colormapCombo);
	connect(colormapCombo, &QComboBox::currentTextChanged, this, [=](const QString &text) {
			auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
			if(!docker)
				return;
			docker->gradType = gradientMap[text];
			docker->replot();
			});
	scaleSpinBox = new QDoubleSpinBox;
	scaleSpinBox->setValue(1);
	scaleSpinBox->setSingleStep(0.5);
	scaleSpinBox->setDecimals(3);
	scaleSpinBox->setMinimum(0.000001);
	toolBar->addWidget(scaleSpinBox);
	connect(scaleSpinBox, &QDoubleSpinBox::valueChanged, this, [=](float val){
			auto docker = dynamic_cast<ProfileDocker*>(mainTab->tabWidget->currentWidget());
			if(!docker)
				return;
			docker->replot(val);
			});

}

void MainWindow::showpProceduresDialog()
{
	if(!mainTab->tabWidget->currentWidget())
		return;
    ProceduresDialog *options = new ProceduresDialog(mainTab->tabWidget, this);
    options->setAttribute(Qt::WA_DeleteOnClose);
    options->show(); 
}

void MainWindow::onOpenFile()
{
	auto filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "/home/zxcv/geofiz");
	//QString filename = "/home/zxcv/geofiz/GPRdata/ExampleDuneProfile/DuneData.DZT";

	if(!filename.length())
		return;

	std::shared_ptr<Profile> prof = std::make_shared<Profile>(filename.toStdString());
	mainTab->addTab(prof);
}
