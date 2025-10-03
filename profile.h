#ifndef PROFILE_H
#define PROFILE_H

#include "formats.h"
#include "qcustomplot.h"
#include "pugixml.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <utility>
#include <map>
#include <fftw3.h>
#include <QWidget>
#include <QMainWindow>

using file_pair = std::pair<std::string, std::string>;// file_pair;
std::ifstream open_both_cases(std::string name, std::string ext);
file_pair split_filename(std::string fname);

struct Profile
{
	std::string path;
	size_t samples;
	size_t traces;
	float timeWindow;
	float *data = nullptr;
	float *timeDomain = nullptr;
	size_t *picks = nullptr;
	std::vector<size_t> marks;
	bool marksOn = false;

	const inline double fs()
	{
		double T = timeWindow*1e-9;
		return samples/T;  
	}

	std::pair<QVector<double>, QVector<double>> prepareWiggleData(size_t, char);
	QCustomPlot* createWiggle(size_t, char type=0);
	std::optional<std::pair<QCustomPlot*, QCPColorMap*>> createRadargram(QCPColorGradient::GradientPreset gradType=QCPColorGradient::gpGrayscale, float scale=1);
	std::shared_ptr<Profile> subtractDcShift(float, float);
	std::shared_ptr<Profile> subtractDewow(float);
	std::shared_ptr<Profile> gainFunction(float, float, float, float);
	std::shared_ptr<Profile> ampltitudesTo0(float, float );
	std::shared_ptr<Profile> xFlip();
	std::shared_ptr<Profile> yFlip();
	std::shared_ptr<Profile> timeCut(float);
	std::shared_ptr<Profile> moveStartTime(float);
	std::shared_ptr<Profile> butterworthFilter(float, float, float, float);

	size_t* naivePicking();
	float* maxSamplePerTrace();
	float maxAmplitude();
	float minAmplitude();
	Profile() {}
	Profile(Profile&);
	Profile(Profile&&);
	Profile(Profile*, float*);
	Profile(std::string);
	Profile(size_t, size_t, float, float*);
	~Profile();
private:
	bool init = false;

	int askForChannelDialog(tagRFHeader*);
	void open_ss(std::string);
	void open_gssi(std::string);
	void open_mala(std::string, bool f=0);
	void read_rad(std::string);
	void read_hd(std::string);
	void readTimeDomain();
	void readMarks(std::ifstream&, int, size_t, tagRFHeader *);
	void detectMarks(float*); 
	std::vector<size_t> readMarksFromDzx(std::string);
	template<class T>
	void read_rd37()
	{
		std::ifstream in(path, std::ios::binary); 
		if(!in)
			throw std::invalid_argument("No rd37 binary file");

		in.seekg(0, std::ios_base::end);
		size_t sz = static_cast<size_t>(in.tellg());
		in.seekg(0, std::ios_base::beg);

		data = read_typed_data<T>(in, sz);
		in.close();

	}
	
	template<class T>
	float* read_typed_data(std::ifstream &in, size_t sz, size_t offset=0)
	{
		float *dt = (float*) fftwf_malloc(sizeof(float)*sz);
		if(!dt)
			throw std::runtime_error("No memory");
		for(int i=0; i<sz; i++)
		{
			if(offset && i%offset == 0)
				in.seekg(offset*sizeof(T), std::ios_base::cur);
			T buf;
			in.read(reinterpret_cast<char*>(&buf), sizeof(T));
			dt[i] = buf;
		}
		return dt;
	}
	
};

class MyQCustomPlot : public QCustomPlot 
{
    Q_OBJECT
public:
	MyQCustomPlot(QWidget *parent=nullptr) : QCustomPlot(parent)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, &QCustomPlot::customContextMenuRequested,
			this, &MyQCustomPlot::showContextMenu);
	}
	
	void showContextMenu(const QPoint &pos)
	{
		QMenu contextMenu(this);

		QAction xTitleAct("Add x axis title", this);
		QAction yTitleAct("Add y axis title", this);
		connect(&xTitleAct, &QAction::triggered, this, &MyQCustomPlot::addXTitleAction);
		connect(&yTitleAct, &QAction::triggered, this, &MyQCustomPlot::addYTitleAction);
		contextMenu.addAction(&xTitleAct);
		contextMenu.addAction(&yTitleAct);
		contextMenu.exec(mapToGlobal(pos));
	}

	void addXTitleAction()
	{
		QDialog dialog;
		dialog.setWindowTitle("X axis title");

		QHBoxLayout *layout = new QHBoxLayout(&dialog);

		QLineEdit *lineEdit = new QLineEdit(&dialog);
		QSpinBox *fontSizeSpin = new QSpinBox(this);
		QPushButton *okButton = new QPushButton("Ok", this);
		fontSizeSpin->setRange(2, 72);
		fontSizeSpin->setValue(10);
		layout->addWidget(lineEdit);
		layout->addWidget(fontSizeSpin);
		layout->addWidget(okButton);
		QObject::connect(lineEdit, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
		QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
		if (dialog.exec() == QDialog::Accepted)
		{
			auto font = QFont("Arial", fontSizeSpin->value());
			this->xAxis2->setLabel(lineEdit->text());
			this->xAxis2->setLabelFont(font);
		}
	}

	void addYTitleAction()
	{
		QDialog dialog;
		dialog.setWindowTitle("Y axis title");

		QHBoxLayout *layout = new QHBoxLayout(&dialog);

		QLineEdit *lineEdit = new QLineEdit(&dialog);
		QSpinBox *fontSizeSpin = new QSpinBox(this);
		QPushButton *okButton = new QPushButton("Ok", this);
		fontSizeSpin->setRange(2, 72);
		fontSizeSpin->setValue(10);
		layout->addWidget(lineEdit);
		layout->addWidget(fontSizeSpin);
		layout->addWidget(okButton);
		QObject::connect(lineEdit, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
		QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
		if (dialog.exec() == QDialog::Accepted)
		{
			auto font = QFont("Arial", fontSizeSpin->value());
			this->yAxis->setLabel(lineEdit->text());
			this->yAxis->setLabelFont(font);
		}
	}

};
#endif
