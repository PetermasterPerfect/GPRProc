#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QMenuBar>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <cstring>
#include <memory>
#include "qcustomplot.h"
#include "profile.h"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
	QCustomPlot *customPlot = new QCustomPlot;//(&window);
	if(argc != 2)
	{
		std::cerr << "Bad arg count\n";
		return -1;
	}

	Profile profile(argv[1]);

	// create graph and assign data to it:
	customPlot->addGraph();
	//customPlot->graph(0)->setData(x, y);
	// give the axes some labels:
	customPlot->xAxis->setLabel("x");
	customPlot->yAxis->setLabel("y");
	customPlot->yAxis->setRangeReversed(true);
	QCPRange samplesRange(1, profile.samples);
	QCPRange tracesRange(1, profile.lastTrace);
	QCPColorMapData mapData(profile.lastTrace, profile.samples, tracesRange, samplesRange);
	for(size_t i=1; i<=profile.lastTrace; i++)
		for(size_t j=1; j<=profile.samples; j++)
			mapData.setData(i, j, profile.data[(i-1)*profile.samples+(j-1)]);
	QCPColorMap map(customPlot->xAxis, customPlot->yAxis);
	map.setData(&mapData);
	map.rescaleDataRange();
	// set axes ranges, so we see all data:
	customPlot->rescaleAxes();
	customPlot->replot();
	customPlot->show();
	window.show();

    return app.exec();
}
