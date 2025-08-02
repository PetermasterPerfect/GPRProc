#ifndef TABBEDWORKSPACEWIDGET_H
#define TABBEDWORKSPACEWIDGET_H

#include <QMainWindow>
#include "DockManager.h"
#include "profile.h"
#include "profiledocker.h"

class TabbedWorkspaceWidget : public QWidget
{
  Q_OBJECT

public:
	typedef struct
	{
	} MainWindowArea;

	explicit TabbedWorkspaceWidget(QString name, QMainWindow* mainWin, QMainWindow* parent);
	ProfileDocker* addTab(std::shared_ptr<Profile>);
	~TabbedWorkspaceWidget() override;

	static TabbedWorkspaceWidget* getInstance(const QString& key);

	QTabWidget* tabWidget;

private slots:
	void on_tabClose(int index); 
 
private:
	const QString name;
	QMainWindow* mainWindow;

	//virtual void closeEvent(QCloseEvent* event) override;

protected:
	static std::map<QString, TabbedWorkspaceWidget*> instances;

signals:
  void created();
  void undoableChange();
  void tabAdded(ads::CDockManager*);

};

#endif
