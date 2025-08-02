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
	bool isWiggled() { return wiggled; }
	void setWiggled(bool val) { wiggled = val; }

	Profile profile;
private:
	QString name;
	bool wiggled = false;
};

#endif
