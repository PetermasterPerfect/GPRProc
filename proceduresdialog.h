#ifndef PROCEDURESDIALOG_H
#define PROCEDURESDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"
#include <functional>
#include <any>

#define TIMEWINDOW(pre, post) QDoubleSpinBox* pre##TimeWindow##post;

static const char *gProceduresNames[] = {"Subtract DC-shif", "Subtract mean (dewow)", "Exponent gain", "Amplitudes to 0", "X(traces) flip", "Y(samples) flip"};

class ProceduresDialog : public QDialog {
    Q_OBJECT
public:
    ProceduresDialog(QTabWidget *tab, QWidget *parent = nullptr);

private slots:
    void onDcShift(bool checked);
    void onDewow(bool checked);
    void onGain(bool checked);
    void onAmplitude0(bool checked);
    void onXFlip(bool checked);
    void onYFlip(bool checked);
	void onPopupUpdate();

private:
	using SlotType = void (ProceduresDialog::*)(bool);
	std::array<SlotType, 6> onProcSlots = {
		&ProceduresDialog::onDcShift,
		&ProceduresDialog::onDewow,
		&ProceduresDialog::onGain,
		&ProceduresDialog::onAmplitude0,
		&ProceduresDialog::onXFlip,
		&ProceduresDialog::onYFlip,
	};
	QTabWidget *tabWidget;

	MyQComboBox* procStepsCombo;
	std::array<QRadioButton*, sizeof(gProceduresNames)/sizeof(char*)> proceduresRadios;
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
	QWidget* createAmplitude0Page();
	QWidget* createXFlipPage();
	QWidget* createYFlipPage();
	void setupStackedOptions();
	QString getProcessingName(ProfileDocker*, QLineEdit*);
	std::shared_ptr<Profile> getCurrentProcessing();
};


#endif
