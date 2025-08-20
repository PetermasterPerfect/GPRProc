#ifndef PROFILE_H
#define PROFILE_H

#include "formats.h"
#include "qcustomplot.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <utility>
#include <map>
#include <fftw3.h>

using file_pair = std::pair<std::string, std::string>;// file_pair;
std::ifstream open_both_cases(std::string name, std::string ext);
file_pair split_filename(std::string fname);

struct Profile
{
	std::string path;
	size_t samples;
	size_t traces;
	double timeWindow;
	double *data = nullptr;
	double *timeDomain = nullptr;
	size_t *picks = nullptr;
	std::vector<size_t> marks;

	std::pair<QVector<double>, QVector<double>> prepareWiggleData(size_t, char);
	QCustomPlot* createWiggle(size_t, char type=0);
	std::optional<std::pair<QCustomPlot*, QCPColorMap*>> createRadargram(QCPColorGradient::GradientPreset gradType=QCPColorGradient::gpGrayscale, double scale=1);
	std::shared_ptr<Profile> subtractDcShift(double, double);
	std::shared_ptr<Profile> subtractDewow(double);
	std::shared_ptr<Profile> gainFunction(double timeStart, double timeEnd, double exponent, double maxVal);
	std::shared_ptr<Profile> ampltitudesTo0(double amplMin, double amplMax);
	std::shared_ptr<Profile> xFlip();
	std::shared_ptr<Profile> yFlip();

	size_t* naivePicking();
	double* maxSamplePerTrace();
	double maxAmplitude();
	double minAmplitude();
	Profile() {}
	Profile(Profile&);
	Profile(Profile&&);
	Profile(Profile*, double*);
	Profile(std::string);
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
	template<class T>
	void read_rd37()
	{
		std::ifstream in(path, std::ios::binary); 
		if(!in)
			throw std::invalid_argument("No rd37 binary file");

		in.seekg(0, std::ios_base::end);
		size_t sz = static_cast<size_t>(in.tellg());
		in.seekg(0, std::ios_base::beg);

		read_typed_data<T>(in, sz);
		in.close();

	}
	
	template<class T>
	void read_typed_data(std::ifstream &in, size_t sz, size_t offset=0)
	{
		data = fftw_alloc_real(sz);
		if(!data)
			throw std::runtime_error("No memory");
		for(int i=0; i<sz; i++)
		{
			if(offset && i%offset == 0)
				in.seekg(offset*sizeof(T), std::ios_base::cur);
			T buf;
			in.read(reinterpret_cast<char*>(&buf), sizeof(T));
			data[i] = buf;
		}
	}
	
};
#endif
