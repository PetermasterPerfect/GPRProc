#include "profiledocker.h"


ProfileDocker::ProfileDocker(QString name, Profile& prof, QWidget* parent) :
	ads::CDockManager(parent), name(name), profile(prof)
{
}

ProfileDocker::~ProfileDocker()
{
}


void ProfileDocker::replot()
{
	for(auto widget : dockWidgets())
	{

		std::cout << "widgets iter\n";
		if(auto plot = dynamic_cast<QCustomPlot*>(widget->widget()))
		{
			QCPColorGradient gradient;
			gradient.loadPreset(gradType);
			radargram2ColorMap[plot]->setGradient(gradient);
			plot->replot();
		}

	}
	std::cout << "---------\n";
}

void ProfileDocker::removeColorMap(QCustomPlot* radargram)
{
	std::cout << "Removed\n";
	if(radargram2ColorMap.find(radargram) != radargram2ColorMap.end())
		radargram2ColorMap.erase(radargram);
}

