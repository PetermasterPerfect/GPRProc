#include "processingstepsdialog.h"

ProcessingStepsDialog::ProcessingStepsDialog(QTabWidget *tab, QWidget *parent)
    : tabWidget(tab), QDialog(parent)
{

    setWindowTitle("Processing steps");
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if (!docker)
		return;
	auto layout = new QVBoxLayout(this);
	table = new QTableWidget(docker->processingSteps.size(), 3, this);
	size_t i = 0;
	for(auto const& x: docker->processingSteps)
	{
		addShowDeleteButtons(x, i);
		table->setItem(i++, 0, new QTableWidgetItem(x.first));
		steps.push_back(x.second);
	}
	table->resizeRowsToContents();
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	//table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	layout->addWidget(table);
	setLayout(layout);

	auto timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [=]() {
			std::cout << "TIMER\n";
			deleteMutex.lock();
			for(auto const& x: docker->processingSteps)
			{
				int idx = stepIndex(x.second);
				if(idx != -1)
				{
					auto showButton = dynamic_cast<QPushButton*>(table->cellWidget(idx, 1));
					bool vis = docker->isProfileVisible(x.second);
					if(vis != showButton->isFlat())
						showButton->setFlat(vis);
				}
				else
				{
					int r = table->rowCount();
					table->insertRow(r);
					addShowDeleteButtons(x, r);
					table->setItem(r, 0, new QTableWidgetItem(x.first));
					steps.push_back(x.second);
				}
			}
			deleteMutex.unlock();
			});
	timer->start(500);
}


void ProcessingStepsDialog::addShowDeleteButtons(std::pair<QString, std::shared_ptr<Profile>> procStep, int row)
{
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if (!docker)
		return;

	auto showButton = new QPushButton("Show", table);
	showButton->setFlat(docker->isProfileVisible(procStep.second));
	auto deleteButton = new QPushButton("Delete", table);
	table->setCellWidget(row, 1, showButton);
	table->setCellWidget(row, 2, deleteButton);

	connect(showButton, &QPushButton::clicked, this, [=](){
		if(showButton->isFlat())
			return;
		auto widget = docker->createDockWidget(procStep.first);
		auto plotPair = procStep.second->createRadargram(docker->gradType, docker->scale);
		if(!plotPair.value().first)
			return;

		connect(widget, &ads::CDockWidget::closed, docker, [=]() {
				docker->removeDockWidget(widget);
				docker->removeColorMap(plotPair.value().first);
			});
		widget->setWidget(plotPair.value().first);
		docker->radargram2ColorMap.insert(plotPair.value());
		docker->addDockWidget(ads::BottomDockWidgetArea, widget);
		showButton->setFlat(false);
			});

	connect(deleteButton, &QPushButton::clicked, this, [=](){
			std::cout << "del start\n";
			deleteMutex.lock();
			if(docker->processingSteps.size() == 1)
				tabWidget->removeTab(tabWidget->currentIndex());
			auto procName = table->item(row, 0)->text();
			table->removeRow(row);
			auto pos = std::find(steps.begin(), steps.end(), docker->processingSteps[procName]);
			if (pos != steps.end())
				steps.erase(pos);
			docker->removeProcessingStep(procName);
			std::cout << "del end\n";
			deleteMutex.unlock();

			});
}

int ProcessingStepsDialog::stepIndex(std::shared_ptr<Profile> prof)
{
	for(int i=0; i<steps.size(); i++)
		if(prof == steps[i])
			return i;
	return -1;
}
