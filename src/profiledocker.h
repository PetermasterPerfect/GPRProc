#ifndef PROFILEDOCKER_H
#define PROFILEDOCKER_H

#include <QMainWindow>
#include "DockManager.h"
#include "profile.h"

template <class Base>
class SubTickTicker : public Base
{
public:
    int subTicks = 0;
protected:
    int getSubTickCount(double /*tickStep*/) override { return subTicks; }
};


class MyQComboBox : public QComboBox
{
    Q_OBJECT
public:
    MyQComboBox(QWidget *parent = nullptr) : QComboBox(parent) {}
	void showPopup() 
	{
	  emit signalPopupShown();
	  QComboBox::showPopup();
	}
signals:
    void signalPopupShown();

};

class ProfileDocker : public ads::CDockManager
{
	Q_OBJECT

	friend class ProceduresDialog;
public:
	ProfileDocker(QString name, QWidget* parent = nullptr);
	~ProfileDocker() override;

	ads::CDockWidget* wiggle = nullptr;
	char wiggleType = 0;
	bool traceNormalization = false;
	bool colorScale = false;
	bool userMarks = false;
	bool depthAxis = false;
	float scale = 1;
	float vel = 0.1;
	float dx = 0;
	float dt = 0;
	float dz = 0;
	char subticks = 5;

	QCPColorGradient::GradientPreset gradType = QCPColorGradient::gpGrayscale;
	std::map<QCustomPlot*, QCPColorMap*> radargram2ColorMap;
	std::map<QString, std::shared_ptr<Profile>> processingSteps;
	std::pair<QString, std::shared_ptr<Profile>> anonymousProc;

	void replot();
	void replot(float);
	void replot(bool);
	void replot(float, bool, bool, bool);
	void replotMarks(bool);
	void replotDepthAxis(bool);
	void removeColorMap(QCustomPlot*);
	bool containsMarks();
	bool isProfileVisible(std::shared_ptr<Profile>);
	void removeProcessingStep(QString);
	void removeProcessingSteps();
	ads::CDockWidget* addRadargramView(std::shared_ptr<Profile>, QString);
	void extendPlot(std::shared_ptr<Profile>, QCustomPlot*, QCPColorMap*);
	void extendPlot(std::shared_ptr<Profile>, QCustomPlot*,QCPColorMap*,
			float, bool, bool, bool, bool);
	std::optional<std::pair<QCustomPlot*, QCPColorMap*>> singlePlot(QWidget*, std::shared_ptr<Profile>);
private:
	QString name;
	void replotColorScale(QCustomPlot*, QCPColorMap*);
	void resetProperties();
	void rescaleAxis(QCPAxis*, float);
};

#endif
