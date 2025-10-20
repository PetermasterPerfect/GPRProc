#ifndef LOADMARKSDIALOG_H
#define LOADMARKSDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"
#include <functional>
#include <any>

class LoadUserMarksDialog : public QDialog {
    Q_OBJECT
public:
    LoadUserMarksDialog(QTabWidget *tab, QAction *act,QWidget *parent = nullptr);
private:
	QTabWidget *tabWidget;
	std::vector<QCheckBox*> profilesChecks;
	QAction *userMarksAct;
	void loadUserMarks();
};
#endif
