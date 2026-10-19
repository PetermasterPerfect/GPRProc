#include "profiledocker.h"


ProfileDocker::ProfileDocker(QString name, QWidget* parent) :
	ads::CDockManager(parent), name(name), anonymousProc("", nullptr)
{
}

ProfileDocker::~ProfileDocker()
{
	std::cout << "~~DEST ProfileDocker~~\n";
	if(anonymousProc.second)
	{
		if(anonymousProc.second->data)
			fftwf_free(anonymousProc.second->data);
		anonymousProc.second->data = nullptr;
	}
}


void ProfileDocker::replot()
{
	replot(scale, traceNormalization, userMarks, depthAxis);
}

void ProfileDocker::replot(float sc)
{
	replot(sc, traceNormalization, userMarks, depthAxis);
}

void ProfileDocker::replot(bool traceNorm)
{
	replot(scale, traceNorm, userMarks, depthAxis);
}

void ProfileDocker::replotMarks(bool marks)
{
	replot(scale, traceNormalization, marks, depthAxis);
}

void ProfileDocker::replotDepthAxis(bool depth)
{
	if(depth)
		depthAxis = false;
	
	replot(scale, traceNormalization, userMarks, depth);
}

void ProfileDocker::replot(float sc, bool traceNorm, bool marks, bool depth)
{
	for(auto widget : dockWidgets())
	{

		if(widget == wiggle)
			continue;

		if(auto plot = dynamic_cast<QCustomPlot*>(widget->widget()))
		{
			std::shared_ptr<Profile> profile;
			if(processingSteps.find(widget->objectName()) != processingSteps.end())
				profile = processingSteps[widget->objectName()];
			else
				profile = anonymousProc.second;

			QCPColorGradient gradient;
			gradient.loadPreset(gradType);
			radargram2ColorMap[plot]->setGradient(gradient);
			extendPlot(profile, plot, radargram2ColorMap[plot], sc, traceNorm, marks, depth, true);
		}

	}

	scale = sc;
	traceNormalization = traceNorm;
	depthAxis = depth;
	if(marks != userMarks)
		userMarks = marks;
}

void ProfileDocker::extendPlot(std::shared_ptr<Profile> profile, QCustomPlot *plot, QCPColorMap *map)
{
	extendPlot(profile, plot, map, scale, traceNormalization, userMarks, depthAxis, false);
}

void ProfileDocker::extendPlot(std::shared_ptr<Profile> profile, QCustomPlot *plot, QCPColorMap *map,
		float sc, bool traceNorm, bool marks, bool depth, bool onCanvas)
{
	auto mapData = map->data();
	if(depth != depthAxis || !onCanvas)
	{
		if(depth)
		{
			plot->yAxis2->setVisible(true);
			plot->yAxis2->setTickLabels(true);
			((MyQCustomPlot*)plot)->setAxisTitle(plot->yAxis2, profile->zTitle);
			plot->yAxis2->setRangeReversed(true);
			plot->yAxis2->setRange(0, vel*profile->timeWindow/2);
		}
		else
		{
			plot->yAxis2->setVisible(false);
			plot->yAxis2->setTickLabels(false);
		}
	}

	if(marks != profile->marksOn || !onCanvas)
	{
		std::cout << "count: " << plot->axisRectCount() << "\n";
		if(marks)
			for(auto mark : profile->marks)
			{
				QCPItemStraightLine *line = new QCPItemStraightLine(plot);
				line->point1->setCoords(mark*profile->distInterval, 0);
				line->point2->setCoords(mark*profile->distInterval, profile->samples-1);
				line->setPen(QPen(Qt::blue, 2, Qt::DashLine));
			}
		else
		{
			size_t n = profile->marks.size();
			while(n)
			{
				if(!plot->itemCount())
					break;
				for(int i=0; i<plot->itemCount(); i++)
					if(auto line = dynamic_cast<QCPItemStraightLine*>(plot->item(i)))
					{
						plot->removeItem(line);
						n--;
						break;
					}
			}
		}
		profile->marksOn = marks;

	}
	if(sc != scale || traceNorm != traceNormalization || !onCanvas)
	{
		float *norm = nullptr;
		if(traceNorm)
			norm = profile->maxSamplePerTrace();
		for(size_t i=0; i<profile->traces; i++)
			for(size_t j=0; j<profile->samples; j++)
			{
				float cell = profile->data[i*profile->samples+j];
				if(norm)
					mapData->setCell(i, j, cell/norm[i]);
				else
					mapData->setCell(i, j, cell);
			}
		map->rescaleDataRange(true);

		for(size_t i=0; i<profile->traces; i++)
			for(size_t j=0; j<profile->samples; j++)
			{
				float cell = profile->data[i*profile->samples+j];
				if(norm)
					mapData->setCell(i, j, (cell/norm[i])*sc);
				else
					mapData->setCell(i, j, cell*sc);
			}

		if(norm)
			delete[] norm;
	}

	rescaleAxis(plot->xAxis, dx);
	rescaleAxis(plot->yAxis, dt);
	rescaleAxis(plot->yAxis2, dz);
	replotColorScale(plot, map);
	plot->replot();
}

std::optional<std::pair<QCustomPlot*, QCPColorMap*>> ProfileDocker::singlePlot(QWidget *parent, std::shared_ptr<Profile> profile)
{
	return profile->createRadargram(parent, gradType, scale);
}

void ProfileDocker::rescaleAxis(QCPAxis *axis, float d)
{
    if (d > 0)
    {
        auto ticker = QSharedPointer<SubTickTicker<QCPAxisTickerFixed>>::create();
        ticker->setTickStep(d);
        ticker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
        ticker->subTicks = subticks;
        axis->setTicker(ticker);
    }
    else
    {
        auto ticker = QSharedPointer<SubTickTicker<QCPAxisTicker>>::create();
        ticker->subTicks = subticks;
        axis->setTicker(ticker);
    }
}

void ProfileDocker::removeColorMap(QCustomPlot* radargram)
{
	std::cout << "Removed\n";
	if(radargram2ColorMap.find(radargram) != radargram2ColorMap.end())
		radargram2ColorMap.erase(radargram);
}


void ProfileDocker::replotColorScale(QCustomPlot *plot, QCPColorMap *colorMap)
{
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


bool ProfileDocker::containsMarks()
{
	auto prof = processingSteps.begin()->second;
	return !prof->marks.empty();
}


bool ProfileDocker::isProfileVisible(std::shared_ptr<Profile> prof)
{
	for(auto widget : dockWidgets())
	{
		if(widget == wiggle)
			continue;
		if(processingSteps.find(widget->objectName())->second == prof)
			return true;
	}
	return false;
}

void ProfileDocker::removeProcessingStep(QString procName)
{
	for(auto widget : dockWidgets())
	{
		if(procName == widget->objectName())
		{
			removeDockWidget(widget);
			break;
		}
	}

	processingSteps.erase(procName);
}

void ProfileDocker::removeProcessingSteps()
{
	for(auto widget : dockWidgets())
		removeDockWidget(widget);


	processingSteps.clear();
	anonymousProc = std::make_pair("", nullptr);
}


void ProfileDocker::resetProperties()
{
	traceNormalization = false;
	userMarks = false;
	scale = 1;
}

ads::CDockWidget* ProfileDocker::addRadargramView(std::shared_ptr<Profile> profile, QString name)
{
	auto widget = new ads::CDockWidget(name, this);
	auto plotPair = profile->createRadargram(this, gradType, scale);
	if(!plotPair)
		return nullptr;

	widget->setWidget(plotPair.value().first);
	widget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, 1);
	auto [plot, colorMap] = plotPair.value();
	radargram2ColorMap.insert({plot, colorMap});
	addDockWidget(ads::TopDockWidgetArea, widget);

	extendPlot(profile, plot, colorMap);
	connect(widget, &ads::CDockWidget::closed, this, [=]() {
				std::cout << "dock widget closing\n";
				removeDockWidget(widget);
				removeColorMap(plotPair.value().first);
				//if(dockWidgets().size() == 0)
				//	resetProperties();
			});


	return widget;
}
