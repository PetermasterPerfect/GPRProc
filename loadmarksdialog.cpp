#include "loadmarksdialog.h"

LoadUserMarksDialog::LoadUserMarksDialog(QTabWidget *tab, QAction *act, QWidget *parent)
    : tabWidget(tab), userMarksAct(act), QDialog(parent)
{
    setWindowTitle("Load marks");
	auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
	if (!docker)
		return;


    QVBoxLayout *checkLayout = new QVBoxLayout(this);
	for (auto &x : docker->processingSteps)
	{
		QCheckBox *box = new QCheckBox(x.first, this);
		profilesChecks.push_back(box);
		checkLayout->addWidget(box);
	}

	if(profilesChecks.size())
		profilesChecks.front()->setChecked(true);

	QPushButton *openButton = new QPushButton("Browse...", this);
	connect(openButton, &QPushButton::clicked, this, &LoadUserMarksDialog::loadUserMarks);
	openButton->setIcon(QIcon::fromTheme("document-open"));
	checkLayout->addWidget(openButton);
}

void LoadUserMarksDialog::loadUserMarks()
{
	try
	{
		QString file = QFileDialog::getOpenFileName(this, "Select File");
		if (!file.isEmpty())
		{
			auto marks = Profile::loadMarksIndexesFromFile(file.toStdString());
			auto docker = dynamic_cast<ProfileDocker*>(tabWidget->currentWidget());
			if (!docker)
				return;
			size_t i = 0;
			for (auto &x : docker->processingSteps)
			{
				if(profilesChecks[i]->isChecked())
					x.second->loadMarksFromVector(marks);
				i++;
			}
			userMarksAct->setEnabled(true);
			this->close();
		}
	}
	catch(const std::runtime_error& e)
	{
        std::cerr << "Cant open the file\n";
	}
	catch(const std::invalid_argument& e) 
	{
        std::cerr << "Invalid argument: no conversion could be performed\n";
    }
    catch(const std::out_of_range& e)
	{
        std::cerr << "Out of range: number too large or too small\n";
    }
}
