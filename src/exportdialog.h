#ifndef EXPORTDIALOG_DIALOG_H
#define EXPORTDIALOG_DIALOG_H
#include "profiledocker.h"

class ExportDialog : public QDialog 
{
    Q_OBJECT

public:
    ExportDialog(QTabWidget *tab, QWidget *parent = nullptr);

	QTabWidget *tabWidget;
private:
};
#endif
