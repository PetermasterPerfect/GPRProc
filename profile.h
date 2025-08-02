#ifndef PROFILE_H
#define PROFILE_H

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
	uint32_t samples;
	uint32_t traces;
	double timeWindow;
	double *data = nullptr;
	double *timeDomain = nullptr;

	QCustomPlot* createWiggle(size_t, char type=0);
	std::optional<std::pair<QCustomPlot*, QCPColorMap*>> createRadargram(double *dt=nullptr, QCPColorGradient::GradientPreset gradType=QCPColorGradient::gpGrayscale, double scale=1);
	double *subtractDcShift(double, double);
	Profile() {}
	Profile(Profile&);
	Profile(Profile&&);
	~Profile();
	Profile(std::string);
private:
	bool init = false;

	void open_ss(std::string);
	void open_gssi(std::string, uint16_t channel=1);
	void open_mala(std::string, bool f=0);
	void read_rad(std::string);
	void read_hd(std::string);
	void read_timeDomain();
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
	void read_typed_data(std::ifstream &in, size_t sz)
	{
		data = fftw_alloc_real(sz/sizeof(T));
		if(!data)
			throw std::runtime_error("No memory");
		for(int i=0; i<sz/sizeof(T); i++)
		{
			T buf;
			in.read(reinterpret_cast<char*>(&buf), sizeof(T));
			data[i] = buf;
		}
	}
	
};
#endif
