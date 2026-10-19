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
#include <limits>
#include <fftw3.h>
#include <QWidget>
#include <QMainWindow>

using file_pair = std::pair<std::string, std::string>;// file_pair;
std::ifstream open_both_cases(std::string name, std::string ext);
file_pair split_filename(std::string fname);

enum BackgroundRemovalType 
{
	WholeTrace,
	PartTrace,
	MeanInside,
	AllInside
};

enum HorizontalScaleType
{
	Stacking,
	Skipping
};

struct Profile
{
	std::string path;
	size_t samples;
	size_t traces;
	float frequency = std::numeric_limits<double>::quiet_NaN();
	float timeWindow = std::numeric_limits<double>::quiet_NaN();
	float distInterval = std::numeric_limits<double>::quiet_NaN();
	float *data = nullptr;
	float *timeDomain = nullptr;
	size_t *picks = nullptr;
	std::vector<size_t> marks;
	bool marksOn = false;
	std::pair<QString, unsigned char> xTitle = std::make_pair("distance [m]", 14);
	std::pair<QString, unsigned char> yTitle = std::make_pair("time [ns]", 14);
	std::pair<QString, unsigned char> zTitle = std::make_pair("depth [m]", 14);

	const inline double fs()
	{
		double T = timeWindow;
		return samples*1e+9/T;  
	}

	const inline double xend()
	{
		return (traces-1)*distInterval;  
	}


	std::pair<QVector<double>, QVector<double>> prepareWiggleData(size_t, char);
	QCustomPlot* createWiggle(size_t, char type, QWidget *parent=nullptr);
	std::optional<std::pair<QCustomPlot*, QCPColorMap*>> createRadargram(QWidget *parent=nullptr, QCPColorGradient::GradientPreset gradType=QCPColorGradient::gpGrayscale, float scale=1);
	std::shared_ptr<Profile> subtractDcShift(float, float);
	std::shared_ptr<Profile> subtractDewow(float);
	std::shared_ptr<Profile> gainFunction(float, float, float, float);
	std::shared_ptr<Profile> ampltitudesTo0(float, float );
	std::shared_ptr<Profile> xFlip();
	std::shared_ptr<Profile> yFlip();
	std::shared_ptr<Profile> timeCut(float);
	std::shared_ptr<Profile> moveStartTime(float);
	std::shared_ptr<Profile> butterworthFilter(float, float);
	// TODO: think about changing size_t argument to int 
	//  QSpinBox return integer value 
	std::shared_ptr<Profile> agc(size_t);
	std::shared_ptr<Profile> backgroundRemoval(float, float, size_t, size_t, char);
	std::shared_ptr<Profile> horizontalScale(int, char);


	size_t* naivePicking();
	float* maxSamplePerTrace();
	float maxAmplitude();
	float minAmplitude();

	void loadMarksFromVector(std::vector<size_t>);
	static std::vector<size_t> loadMarksIndexesFromFile(std::string);
	std::vector<std::pair<double, double>> readGPSFromDzg();

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
	std::pair<size_t, size_t> timeRangeToIndexes(float, float);
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


	std::shared_ptr<Profile> horizontalScaleStack(int);
	std::shared_ptr<Profile> horizontalScaleSkip(int);
	
};

class MyQCustomPlot : public QCustomPlot 
{
    Q_OBJECT
	Profile *prof;

	void changeTitle(QCPAxis *axis, QString dialogTitle, std::pair<QString, unsigned char> &textInfo)
	{
		QDialog dialog;
		dialog.setWindowTitle(dialogTitle);

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
			auto title = lineEdit->text();
			auto fontSz = fontSizeSpin->value();
			textInfo = std::make_pair(title, fontSz); 
			setAxisTitle(axis, textInfo);
			
		}
		this->replot();
	}

public:
	MyQCustomPlot(Profile *p, QWidget *parent=nullptr) : QCustomPlot(parent)
	{
		prof = p;
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, &QCustomPlot::customContextMenuRequested,
			this, &MyQCustomPlot::showContextMenu);
	}
	
	void showContextMenu(const QPoint &pos)
	{
		QMenu contextMenu(this);

		QAction xTitleAct("Add x axis title", this);
		QAction yTitleAct("Add y axis title", this);
		QAction zTitleAct("Add z axis title", this);
		connect(&xTitleAct, &QAction::triggered, this, &MyQCustomPlot::addXTitleAction);
		connect(&yTitleAct, &QAction::triggered, this, &MyQCustomPlot::addYTitleAction);
		connect(&zTitleAct, &QAction::triggered, this, &MyQCustomPlot::addZTitleAction);
		contextMenu.addAction(&xTitleAct);
		contextMenu.addAction(&yTitleAct);
		contextMenu.addAction(&zTitleAct);
		contextMenu.exec(mapToGlobal(pos));
	}

	void addXTitleAction()
	{
		changeTitle(xAxis2,  "X axis title",prof->xTitle);
	}

	void addYTitleAction()
	{
		changeTitle(yAxis,"Y axis title", prof->yTitle);
	}

	void addZTitleAction()
	{
		changeTitle(yAxis2,"Z axis title", prof->zTitle);
	}


	void setAxisTitle(QCPAxis *axis, std::pair<QString, unsigned char> textInfo)
	{
		auto [title, sz] = textInfo; 
		auto font = QFont("Arial", sz);
		axis->setLabel(title);
		axis->setLabelFont(font);
	}

};
#endif
