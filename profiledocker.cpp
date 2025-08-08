#include "profiledocker.h"


ProfileDocker::ProfileDocker(QString name, QWidget* parent) :
	ads::CDockManager(parent), name(name), anonymousProc("", nullptr)
{
}

ProfileDocker::~ProfileDocker()
{
	if(anonymousProc.second)
	{
		if(anonymousProc.second->data)
			fftw_free(anonymousProc.second->data);
		anonymousProc.second->data = nullptr;
	}
}


void ProfileDocker::replot()
{
	replot(scale, traceNormalization, userMarks);
}

void ProfileDocker::replot(double sc)
{
	replot(sc, traceNormalization, userMarks);
}

void ProfileDocker::replot(bool traceNorm)
{
	replot(scale, traceNorm, userMarks);
}

void ProfileDocker::replotMarks(bool marks)
{
	replot(scale, traceNormalization, marks);
}

void ProfileDocker::replot(double sc, bool traceNorm, bool marks)
{
	for(auto widget : dockWidgets())
	{

		if(widget == wiggle)
			continue;

		if(auto plot = dynamic_cast<QCustomPlot*>(widget->widget()))
		{
			QCPColorGradient gradient;
			gradient.loadPreset(gradType);
			radargram2ColorMap[plot]->setGradient(gradient);
			auto mapData = radargram2ColorMap[plot]->data();
			std::shared_ptr<Profile> profile;
			if(processingSteps.find(widget->objectName()) != processingSteps.end())
				profile = processingSteps[widget->objectName()];
			else
				profile = anonymousProc.second;

			if(marks != userMarks)
			{
				if(marks)
					for(auto mark : profile->marks)
					{
						QCPItemStraightLine *line = new QCPItemStraightLine(plot);
						line->point1->setCoords(mark, 0);
						line->point2->setCoords(mark, profile->samples-1);
						line->setPen(QPen(Qt::blue, 2, Qt::DashLine));
					}
				else
				{
					size_t n = profile->marks.size();
					while(n)
						for(int i=0; i<plot->itemCount(); i++)
							if(auto line = dynamic_cast<QCPItemStraightLine*>(plot->item(i)))
							{
								plot->removeItem(line);
								n--;
								break;
							}
				}

				userMarks = marks;
			}
			if(sc != scale || traceNorm != traceNormalization)
			{
				double *norm = nullptr;
				if(traceNorm)
					norm = profile->maxSamplePerTrace();
				for(size_t i=0; i<profile->traces; i++)
					for(size_t j=0; j<profile->samples; j++)
					{
						double cell = profile->data[i*profile->samples+j];
						if(norm)
							mapData->setCell(i, j, cell/norm[i]);
						else
							mapData->setCell(i, j, cell);
					}
				radargram2ColorMap[plot]->rescaleDataRange(true);

				for(size_t i=0; i<profile->traces; i++)
					for(size_t j=0; j<profile->samples; j++)
					{
						double cell = profile->data[i*profile->samples+j];
						if(norm)
							mapData->setCell(i, j, (cell/norm[i])*sc);
						else
							mapData->setCell(i, j, cell*sc);
					}

				if(norm)
					delete[] norm;
			}

			replotColorScale(plot);
			plot->replot();
		}

	}

	scale = sc;
	traceNormalization = traceNorm;
}

void ProfileDocker::removeColorMap(QCustomPlot* radargram)
{
	std::cout << "Removed\n";
	if(radargram2ColorMap.find(radargram) != radargram2ColorMap.end())
		radargram2ColorMap.erase(radargram);
}


void ProfileDocker::replotColorScale(QCustomPlot *plot)
{
	auto colorMap = radargram2ColorMap[plot];
	if(auto colorSc = plot->plotLayout()->element(0, 1))
	{
		plot->plotLayout()->remove(colorSc);
		plot->plotLayout()->simplify();
	}
	if(colorScale)
	{
		QCPColorScale *colorSc = new QCPColorScale(plot);
		QCPColorGradient gradient;
		gradient.loadPreset(gradType);
		colorSc->setGradient(gradient);
		colorSc->setDataRange(colorMap->dataRange());
		colorMap->setColorScale(colorSc);
		plot->plotLayout()->addElement(0, 1, colorSc);
	}
}
