#ifndef PROFILEDOCKER_H
#define PROFILEDOCKER_H

#include <QMainWindow>
#include "DockManager.h"
#include "profile.h"

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
	float scale = 1;
	QCPColorGradient::GradientPreset gradType = QCPColorGradient::gpGrayscale;
	std::map<QCustomPlot*, QCPColorMap*> radargram2ColorMap;
	std::map<QString, std::shared_ptr<Profile>> processingSteps;
	std::pair<QString, std::shared_ptr<Profile>> anonymousProc;

	void replot();
	void replot(float);
	void replot(bool);
	void replot(float, bool, bool);
	void replotMarks(bool);
	void removeColorMap(QCustomPlot*);
	bool containsMarks();
	bool isProfileVisible(std::shared_ptr<Profile>);
	void removeProcessingStep(QString);
	void removeProcessingSteps();
	ads::CDockWidget* addRadargramView(std::shared_ptr<Profile>, QString);
private:
	QString name;
	void replotColorScale(QCustomPlot*);
};

#endif
