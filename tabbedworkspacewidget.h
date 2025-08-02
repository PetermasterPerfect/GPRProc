#ifndef TABBEDWORKSPACEWIDGET_H
#define TABBEDWORKSPACEWIDGET_H

#include <QMainWindow>
#include "DockManager.h"
#include "profile.h"

class TabbedWorkspaceWidget : public QWidget
{
  Q_OBJECT

public:
	typedef struct
	{
	} MainWindowArea;

	explicit TabbedWorkspaceWidget(QString name, QMainWindow* mainWin, QMainWindow* parent);
	ads::CDockManager* addTab(Profile);
	~TabbedWorkspaceWidget() override;

	static TabbedWorkspaceWidget* getInstance(const QString& key);

	QTabWidget* tabWidget;

private slots:
	void on_tabClose(int index); 
 
private:
	const QString name;
	QMainWindow* mainWindow;
	Profile profile;

	//virtual void closeEvent(QCloseEvent* event) override;

protected:
	static std::map<QString, TabbedWorkspaceWidget*> instances;

signals:
  void created();
  void undoableChange();
  void tabAdded(ads::CDockManager*);

};

#endif
