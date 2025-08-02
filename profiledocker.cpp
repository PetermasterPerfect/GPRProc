#include "profiledocker.h"


ProfileDocker::ProfileDocker(QString name, Profile& prof, QWidget* parent) :
	ads::CDockManager(parent), name(name), profile(prof), anonymousProc("", nullptr)
{
}

ProfileDocker::~ProfileDocker()
{
	if(anonymousProc.second)
		fftw_free(anonymousProc.second);
	anonymousProc.second = nullptr;
}


void ProfileDocker::replot()
{
	replot(scale, traceNormalization);
}

void ProfileDocker::replot(double sc)
{
	replot(sc, traceNormalization);
}

void ProfileDocker::replot(bool traceNorm)
{
	replot(scale, traceNorm);
}

void ProfileDocker::replot(double sc, bool traceNorm)
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
			double *data;
			if(processingSteps.find(widget->objectName()) != processingSteps.end())
				data = processingSteps[widget->objectName()];
			else
				data = anonymousProc.second;

			if(sc != scale || traceNorm != traceNormalization)
			{
				double *norm = traceNorm ? profile.maxSamplePerTrace() : new double[profile.traces]{};
				for(size_t i=0; i<profile.traces; i++)
					for(size_t j=0; j<profile.samples; j++)
					{
						double cell = data[i*profile.samples+j];
						mapData->setCell(i, j, cell-norm[i]);
					}
				radargram2ColorMap[plot]->rescaleDataRange(true);

				for(size_t i=0; i<profile.traces; i++)
					for(size_t j=0; j<profile.samples; j++)
					{
						double cell = data[i*profile.samples+j];
						mapData->setCell(i, j, (cell-norm[i])*sc);
					}

				delete[] norm;
			}
			plot->replot();
		}

	}

	scale = sc;
	traceNormalization = traceNorm;
	std::cout << "---------\n";
}

void ProfileDocker::removeColorMap(QCustomPlot* radargram)
{
	std::cout << "Removed\n";
	if(radargram2ColorMap.find(radargram) != radargram2ColorMap.end())
		radargram2ColorMap.erase(radargram);
}

