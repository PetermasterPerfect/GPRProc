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
	QCPColorGradient::GradientPreset gradType = QCPColorGradient::gpGrayscale;
	std::map<QCustomPlot*, QCPColorMap*> radargram2ColorMap;

	void replot();
	void removeColorMap(QCustomPlot*);
private:
	QString name;
};

#endif
