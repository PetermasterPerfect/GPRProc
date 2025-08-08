#ifndef PROCEDURESDIALOG_H
#define PROCEDURESDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"
#include <functional>
#include <any>

#define TIMEWINDOW(pre, post) QDoubleSpinBox* pre##TimeWindow##post;

static const char *proceduresNames[] = {"Subtract DC-shif", "Subtract mean (dewow)", "Gain Function"};



class ProceduresDialog : public QDialog {
    Q_OBJECT
public:
    ProceduresDialog(QTabWidget *tab, QWidget *parent = nullptr);

private slots:
    void onDcShift(bool checked);
    void onDewow(bool checked);
    void onGain(bool checked);
	void onPopupUpdate();

private:
	QTabWidget *tabWidget;

	MyQComboBox* procStepsCombo;
	std::array<QRadioButton*, sizeof(proceduresNames)/sizeof(char*)> proceduresRadios;
    QStackedWidget *stack;
	/*TIMEWINDOW(dc, 1);
	TIMEWINDOW(dc, 2);
	TIEWINDOW(dewow, 1);*/
	std::any procedur;

	void apply(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyProc(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyBase(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void addProcessing(ProfileDocker*, QPushButton*);

	QWidget* createDcShiftPage();
	QWidget* createDewowPage();
	QWidget* createGainPage();
	void setupStackedOptions();
	QString getProcessingName(ProfileDocker*, QLineEdit*);
	std::shared_ptr<Profile> getCurrentProcessing();
};
#endif
