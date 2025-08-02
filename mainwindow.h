#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "DockManager.h"
#include "profile.h"
#include "tabbedworkspacewidget.h"
#include "proceduresdialog.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(char*);

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

public slots:
    void onOpenFile();
    void print();
    void undo();
    void redo();
	void wiggleView();
	void traceNormalization();
	void colorScale();

private:
    void createActions();
    void createMenus();
	void createToolbar();
	void removeWiggle(ProfileDocker*);
	void setUpWiggle(ProfileDocker*, size_t, int idx=0);
	void showpProceduresDialog();

	TabbedWorkspaceWidget *mainTab;
    QMenu *fileMenu;
	QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *processingMenu;
	QToolBar* toolBar;
    QAction *openAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
	QAction *wiggleViewAct;
	QAction *traceNormalizationAct;
	QAction *colorScaleAct;
	QAction *proceduresAct;
	QComboBox *colormapCombo;
	QDoubleSpinBox *scaleSpinBox;


	std::unordered_map<QString, QCPColorGradient::GradientPreset> gradientMap = {
    {"Gray", QCPColorGradient::gpGrayscale},
    {"Hot",       QCPColorGradient::gpHot},
    {"Cold",      QCPColorGradient::gpCold},
    {"Night",     QCPColorGradient::gpNight},
    {"Candy",     QCPColorGradient::gpCandy},
    {"Geography", QCPColorGradient::gpGeography},
    {"Ion",       QCPColorGradient::gpIon},
    {"Thermal",   QCPColorGradient::gpThermal},
    {"Polar",     QCPColorGradient::gpPolar},
	{"Spectrum",  QCPColorGradient::gpSpectrum},
    {"Jet",       QCPColorGradient::gpJet},
    {"Hues",      QCPColorGradient::gpHues}};
};

#endif
