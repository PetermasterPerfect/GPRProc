CLASS_DEF = """#ifndef PROCEDURESDIALOG_H
#define PROCEDURESDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"
#include <functional>
#include <any>


static const char *gProceduresNames[] = {}; //proc names

class ProceduresDialog : public QDialog {{
    Q_OBJECT
public:
    ProceduresDialog(QTabWidget *tab, QWidget *parent = nullptr);

private slots:
        {} // slots 
	void onPopupUpdate();

private:
	using SlotType = void (ProceduresDialog::*)(bool);
	std::array<SlotType, {}> onProcSlots = {{
	    {} // slots functions	
	}};
	QTabWidget *tabWidget;

	MyQComboBox* procStepsCombo;
	std::array<QRadioButton*, sizeof(gProceduresNames)/sizeof(char*)> proceduresRadios;
    QStackedWidget *stack;
	std::any procedur;

	void apply(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyProc(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void applyBase(ProfileDocker*, std::shared_ptr<Profile>, QString);
	void addProcessing(ProfileDocker*, QPushButton*);

        {} // create pages	
	void setupStackedOptions();
	QString getProcessingName(ProfileDocker*, QLineEdit*);
	std::shared_ptr<Profile> getCurrentProcessing();
}};

#endif"""
