#include "profiledocker.h"


ProfileDocker::ProfileDocker(QString name, Profile& prof, QWidget* parent) :
	ads::CDockManager(parent), name(name), profile(prof)
{
}

ProfileDocker::~ProfileDocker()
{
}
