#ifndef PROFILEDOCKER_H
#define PROFILEDOCKER_H

#include <QMainWindow>
#include "DockManager.h"
#include "profile.h"

class ProfileDocker : public ads::CDockManager
{
	Q_OBJECT

public:
	ProfileDocker(QString name, Profile& prof, QWidget* parent = nullptr);
	~ProfileDocker();

	Profile profile;
	bool wiggle = false;
	char wiggleType = 0;
	bool traceNormalization = false;
	double scale = 1;
	QCPColorGradient::GradientPreset gradType = QCPColorGradient::gpGrayscale;
	std::map<QCustomPlot*, QCPColorMap*> radargram2ColorMap;
	std::map<QString, double*> processingSteps;
	std::pair<QString, double*> anonymousProc;

	void replot();
	void replot(double);
	void replot(bool);
	void replot(double, bool);
	void removeColorMap(QCustomPlot*);
private:
	QString name;
};

#endif
