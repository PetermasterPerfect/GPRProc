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
	replot(scale);
}

void ProfileDocker::replot(double sc)
{
	for(auto widget : dockWidgets())
	{

		std::cout << "widgets iter\n";
		if(auto plot = dynamic_cast<QCustomPlot*>(widget->widget()))
		{
			QCPColorGradient gradient;
			gradient.loadPreset(gradType);
			radargram2ColorMap[plot]->setGradient(gradient);
			auto mapData = radargram2ColorMap[plot]->data();
			std::cout << "key: " << mapData->keySize() << "\n";
			std::cout << "value: " << mapData->valueSize() << "\n";
			if(sc != scale)
			{
				for(size_t i=0; i<profile.traces; i++)
					for(size_t j=0; j<profile.samples; j++)
					{
						double cell = profile.data[i*profile.samples+j];
						mapData->setCell(i, j, cell*scale);
					}
			}
			plot->replot();
		}

	}

	scale = sc;
	std::cout << "---------\n";
}

void ProfileDocker::removeColorMap(QCustomPlot* radargram)
{
	std::cout << "Removed\n";
	if(radargram2ColorMap.find(radargram) != radargram2ColorMap.end())
		radargram2ColorMap.erase(radargram);
}

