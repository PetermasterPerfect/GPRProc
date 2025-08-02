#ifndef PROCEDURESDIALOG_H
#define PROCEDURESDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"
#include <functional>
#include <any>

#define TIMEWINDOW(pre, post) QDoubleSpinBox* pre##TimeWindow##post;

static const char *proceduresNames[] = {"Subtract DC-shif", "Subtract mean (dewow)"};



class ProceduresDialog : public QDialog {
    Q_OBJECT
public:
    ProceduresDialog(QTabWidget *tab, QWidget *parent = nullptr);

private slots:
    void onDcShift(bool checked);
    void onDewow(bool checked);
	void onPopupUpdate();

private:
	QTabWidget *tabWidget;

	MyQComboBox* procStepsCombo;
	std::array<QRadioButton*, 2> proceduresRadios;
    QStackedWidget *stack;
	TIMEWINDOW(dc, 1);
	TIMEWINDOW(dc, 2);
	TIMEWINDOW(dewow, 1);

	void apply(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyProc(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyBase(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void addProcessing(ProfileDocker*, QPushButton*);

	void procDc();
	QWidget* createDcShiftPage();
	QWidget* createDewowPage();
	void setupStackedOptions();
	QString getProcessingName(ProfileDocker*, QLineEdit*);
	std::shared_ptr<Profile> getCurrentProcessing();

	std::any procedur;
};
#endif
