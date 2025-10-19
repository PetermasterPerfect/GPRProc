#ifndef PROCEDURESDIALOG_H
#define PROCEDURESDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"
#include <functional>
#include <any>


static const char *gProceduresNames[] = {"Subtract DC-shift", "Subtract mean (dewow)", "Exponent gain", "Amplitudes to 0", "X(traces) flip", "Y(samples) flip", "Time cut", "Move start time", "Butterworth filter", "Automatic gain control (AGC)", "Background removal", "Horizontal scale"}; //proc names

class ProceduresDialog : public QDialog {
    Q_OBJECT
public:
    ProceduresDialog(QTabWidget *tab, QWidget *parent = nullptr);

private slots:
        void onDcshift(bool checked);
	void onDewow(bool checked);
	void onGain(bool checked);
	void onAmplitudesto0(bool checked);
	void onXflip(bool checked);
	void onYflip(bool checked);
	void onTimecut(bool checked);
	void onMovestarttime(bool checked);
	void onButterworthfilter(bool checked);
	void onAgc(bool checked);
	void onBackgroundremoval(bool checked);
	void onHorizontalscale(bool checked); // slots 
	void onPopupUpdate();

private:
	using SlotType = void (ProceduresDialog::*)(bool);
	std::array<SlotType, 12> onProcSlots = {
	    	&ProceduresDialog::onDcshift,
		&ProceduresDialog::onDewow,
		&ProceduresDialog::onGain,
		&ProceduresDialog::onAmplitudesto0,
		&ProceduresDialog::onXflip,
		&ProceduresDialog::onYflip,
		&ProceduresDialog::onTimecut,
		&ProceduresDialog::onMovestarttime,
		&ProceduresDialog::onButterworthfilter,
		&ProceduresDialog::onAgc,
		&ProceduresDialog::onBackgroundremoval,
		&ProceduresDialog::onHorizontalscale // slots functions	
	};
	QTabWidget *tabWidget;

	MyQComboBox* procStepsCombo;
	std::array<QRadioButton*, sizeof(gProceduresNames)/sizeof(char*)> proceduresRadios;
    QStackedWidget *stack;
	std::any procedur;

	void apply(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyProc(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyBase(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void addProcessing(ProfileDocker*, QPushButton*);

        QWidget* createDcshift();
	QWidget* createDewow();
	QWidget* createGain();
	QWidget* createAmplitudesto0();
	QWidget* createXflip();
	QWidget* createYflip();
	QWidget* createTimecut();
	QWidget* createMovestarttime();
	QWidget* createButterworthfilter();
	QWidget* createAgc();
	QWidget* createBackgroundremoval();
	QWidget* createHorizontalscale(); // create pages	
	void setupStackedOptions();
	QString getProcessingName(ProfileDocker*, QLineEdit*);
	std::shared_ptr<Profile> getCurrentProcessing();
};

#endif