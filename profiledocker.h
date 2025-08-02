#ifndef PROFILEDOCKER_H
#define PROFILEDOCKER_H

#include <QMainWindow>
#include "DockManager.h"
#include "profile.h"

class ProfileDocker : public ads::CDockManager
{
	Q_OBJECT

public:
	ProfileDocker(QString name, Profile& prof, QWidget* parent = nullptr);
	~ProfileDocker();

	Profile profile;
	bool wiggle = false;
	char type = 0;
private:
	QString name;
};

#endif
