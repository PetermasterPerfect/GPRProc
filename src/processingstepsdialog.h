#ifndef PROCESSINGSTEPSDIALOG_DIALOG_H
#define PROCESSINGSTEPSDIALOG_DIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QStackedWidget>
#include "profiledocker.h"

class ProcessingStepsDialog : public QDialog 
{
    Q_OBJECT
public:
    ProcessingStepsDialog(QTabWidget *tab, QWidget *parent = nullptr);

	QTabWidget *tabWidget;
private:
	QMutex deleteMutex;
	QTableWidget *table;
	std::vector<std::shared_ptr<Profile>> steps;
	int stepIndex(std::shared_ptr<Profile>);
	void addShowDeleteButtons(std::pair<QString, std::shared_ptr<Profile>>, int);
};
#endif
