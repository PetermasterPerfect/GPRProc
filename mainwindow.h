#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "DockManager.h"
#include "profile.h"
#include "tabbedworkspacewidget.h"

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

private:
    void createActions();
    void createMenus();
	void createGraph(std::string, QCustomPlot*);
	void createWiggle(size_t, Profile);

	//QCustomPlot *customPlot = nullptr;
	bool profileSet = false;
	ads::CDockManager* dockManager = nullptr;
	ads::CDockWidget* dockWidget = nullptr;
	ads::CDockWidget* dockWidget1 = nullptr;

	TabbedWorkspaceWidget *mainTab;
    QMenu *fileMenu;
	QMenu *editMenu;
    QMenu *viewMenu;
    QAction *openAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
	QAction *wiggleViewAct;

	//QShortcut open_shortcut;
};

#endif
