#ifndef PROCEDURESDIALOG_H
#define PROCEDURESDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"

class ProceduresDialog : public QDialog {
    Q_OBJECT
public:
    ProceduresDialog(QTabWidget *tab, QWidget *parent = nullptr);

private slots:
    void onDcShift(bool checked);
    void onDewow(bool checked);

private:
	QTabWidget *tabWidget;

    QRadioButton *dcShiftRadio;
    QRadioButton *dewowRadio;
    QStackedWidget *stack;

	QWidget* createDcShiftPage();
	QWidget* createDewowPage();
	void setupStackedOptions();
};
#endif
