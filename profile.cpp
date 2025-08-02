#include "profile.h"
#include "formats.h"
#include <cmath>

file_pair split_filename(std::string fname)
{
	size_t dot_pos = fname.find_last_of('.');
	if(std::string::npos == dot_pos)
		throw std::invalid_argument("No extension in file");
	std::string name = fname.substr(0, dot_pos);
	std::string ext = fname.substr(dot_pos+1);
	std::cout << "name: " << name << ", ext: " << ext << "\n";
	return file_pair(name, ext);
}

std::ifstream open_both_cases(std::string name, std::string ext)
{
	std::ifstream in(name+ext, std::ios::binary);
	if(!in)
		in = std::ifstream(name+boost::algorithm::to_lower_copy(ext), std::ios::binary);
	return in;
}


Profile::Profile(std::string p): path(p)
{
	auto path_pair = split_filename(path);
	std::string name = path_pair.first;
	std::string ext = path_pair.second;

	if(boost::algorithm::to_lower_copy(ext) == "rd3")
	 	open_mala(name);
	else if(boost::algorithm::to_lower_copy(ext) == "rd7")
	 	open_mala(name, 1);
	else if(boost::algorithm::to_lower_copy(ext) == "dzt")
		open_gssi(name);
	else if(boost::algorithm::to_lower_copy(ext) == "dt1")
		std::cout << "ok;";
		//load dt1
	else
		std::cout << "Unsupport extension\n";
	
	init = true;
}

Profile::Profile(Profile& prof)
{
	if(!prof.init)
		return;
	path = prof.path;
	samples = prof.samples;
	traces = prof.traces;
	timeWindow = prof.timeWindow;
	data = fftw_alloc_real(samples*traces);
	if(!data)
		throw std::runtime_error("Memory allocation error");
	for(size_t i=0; i<samples*traces; i++)
		data[i] = prof.data[i];

	timeDomain = new double[samples];
	for(unsigned i=0; i<samples; i++)
		timeDomain[i] = prof.timeDomain[i];

	init = true;
}


Profile::Profile(Profile&& prof)
{
	if(!prof.init)
		return;
	path = prof.path;
	samples = prof.samples;
	traces = prof.traces;
	timeWindow = prof.timeWindow;
	data = prof.data;
	prof.data = nullptr;
	timeDomain = prof.timeDomain;
	prof.timeDomain = nullptr;

	init = true;
}

Profile::~Profile()
{
	if(data)
		fftw_free(data);
	data = nullptr;

	if(timeDomain)
		delete[] timeDomain;
	timeDomain = nullptr;
}

QCustomPlot* Profile::createWiggle(size_t n, char type)
{
	if(!init)
		return nullptr;

	QVector<double> x(samples), y(samples);
	QCustomPlot *wigglePlot = new QCustomPlot();
	switch(type)
	{
	case 0: // trace
		for (int i=0; i<samples; ++i)
		{
			x[i] = i;
			y[i] = data[(n-1)*samples+i];
		}
		break;
	case 1: // amplitude
	case 2:
		fftw_plan p;
		fftw_complex *fourier = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * traces*samples);
		double *trace = fftw_alloc_real(samples);
		for (int i=0; i<samples; ++i)
			trace[i] = data[(n-1)*samples+i];

		p = fftw_plan_dft_r2c_1d(samples, trace, fourier, FFTW_ESTIMATE);
		if(!fourier)
			return nullptr;
		fftw_execute(p);
		for (int i=0; i<samples/2; ++i)
		{
			x[i] = i;
			if(type == 1)
				y[i] = sqrt(pow(fourier[i][0], 2) + pow(fourier[i][1], 2));
			else
				y[i] = atan2(fourier[i][1], fourier[i][0]);
		}
		fftw_destroy_plan(p);
		fftw_free(fourier);
		fftw_free(trace);
		break;
	}

	wigglePlot->addGraph();
	wigglePlot->graph(0)->setData(x, y);
	wigglePlot->xAxis->setLabel("x");
	wigglePlot->yAxis->setLabel("y");
	wigglePlot->rescaleAxes();
	wigglePlot->replot();
	return wigglePlot;
}

std::optional<std::pair<QCustomPlot*, QCPColorMap*>> Profile::createRadargram(double *dt, QCPColorGradient::GradientPreset gradType, double scale)
{
	if(!init)
		return std::nullopt;
	if(!dt)
		dt = data;

	QCustomPlot *imagePlot = new QCustomPlot();

	imagePlot->addGraph();
	imagePlot->xAxis->setLabel("x");
	imagePlot->yAxis->setLabel("y");
	imagePlot->yAxis->setRangeReversed(true);
	QCPRange *samplesRange = new QCPRange(1, samples);
	QCPRange *tracesRange = new QCPRange(1, traces);
	QCPColorGradient gradient;
	gradient.loadPreset(gradType);
	QCPColorMapData *mapData = new QCPColorMapData(traces, samples, *tracesRange, *samplesRange);
	for(size_t i=1; i<=traces; i++)
		for(size_t j=1; j<=samples; j++)
			mapData->setData(i, j, dt[(i-1)*samples+(j-1)]*scale);
	QCPColorMap *map = new QCPColorMap(imagePlot->xAxis, imagePlot->yAxis);
	map->setData(mapData);
	map->setGradient(gradient);
	map->rescaleDataRange();
	imagePlot->rescaleAxes();
	imagePlot->replot();
	delete samplesRange;
	delete tracesRange;
	return std::make_optional(std::make_pair(imagePlot, map));;
}

void Profile::open_gssi(std::string name, uint16_t channel)
{
	std::ifstream in = open_both_cases(name, ".DZT"); 
	if(!in)
		throw std::invalid_argument("No dzt file");

	DztHdrStruct hdr;
	in.read(reinterpret_cast<char*>(&hdr), sizeof(DztHdrStruct));
	if(channel <= 0 || channel > hdr.rh_nchan)
		throw std::invalid_argument("Bad channel");

	samples = hdr.rh_nsamp;
	timeWindow = hdr.rh_range;

	in.seekg(0, std::ios_base::end);
	size_t offset = sizeof(DztHdrStruct)*hdr.rh_nchan;
	size_t sz = static_cast<size_t>(in.tellg());
	if(sz <= offset)
		throw std::invalid_argument("Bad offset");
	sz -= offset;
	traces = sz/(hdr.rh_bits/8)/samples;
	in.seekg(offset, std::ios_base::beg);
	//offset/=hdr.rh_bits/8;
	if(hdr.rh_bits == 8)
		read_typed_data<uint8_t>(in, sz);
	else if(hdr.rh_bits == 16)
	{
		read_typed_data<uint16_t>(in, sz);
		for(int i=0; i<sz/2; i++)
			data[i] = data[i] - pow(2, 16)/2;
	}
	else if(hdr.rh_bits == 32)
		read_typed_data<uint32_t>(in, sz);

	read_timeDomain();
}


void Profile::read_hd(std::string name)
{
	std::ifstream in = open_both_cases(name, ".HD"); 
	if(!in)
		throw std::invalid_argument("No hd header file");
	std::string line;
	while(std::getline(in, line)) 
	{
		size_t sep = line.find(" = ");
		if(sep == std::string::npos)
			continue;
		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep+1);	
		if(key == "SAMPLES")
		{
			samples = std::stol(value);
			if(samples <= 0)
			{
				in.close();
				throw std::invalid_argument("Bad header info");
			}
		}
		else if(key == "NUMBER OF TRACES")
		{
			traces = std::stol(value);
			if(traces <= 0)
			{
				in.close();
				throw std::invalid_argument("Bad header info");
			}
		}
	}
	in.close();
}

void Profile::open_mala(std::string name, bool f)
{
	read_rad(name);
	if(!f)
		read_rd37<short>();
	else
		read_rd37<int>();

	read_timeDomain();
	std::cout << "Samples: " << samples << " , traces: " << traces << ", timewindow: " << timeWindow << "\n"; 
}


void Profile::read_timeDomain()
{
	timeDomain = new double[samples];
	if(!timeDomain)
		throw std::runtime_error("No memory");

	double timeIt = timeWindow/samples;
	double curTime = 0;
	for(unsigned i=0; i<samples; i++)
	{
		timeDomain[i] = curTime;
		curTime += timeIt;
	}
}

void Profile::read_rad(std::string name)
{
	std::ifstream in = open_both_cases(name, ".RAD"); 
	if(!in)
		throw std::invalid_argument("No rad header file");

	std::string line;
	//Header ret;
	while(std::getline(in, line)) 
	{
		size_t sep = line.find(':');
		if(sep == std::string::npos)
			continue;
			//throw std::invalid_argument("Bad line");
		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep+1);	
		if(key == "SAMPLES")
		{
			samples = std::stol(value);
			if(samples <= 0)
			{
				in.close();
				throw std::invalid_argument("Bad header info");
			}
		}
		else if(key == "LAST TRACE")
		{
			traces = std::stol(value);
			if(traces <= 0)
			{
				in.close();
				throw std::invalid_argument("Bad header info");
			}
		}
		else if(key == "TIMEWINDOW")
		{
			timeWindow = std::stod(value);
			if(timeWindow <= 0)
			{
				in.close();
				throw std::invalid_argument("Bad header info");
			}
		}
	}
	in.close();
}


double* Profile::subtractDcShift(double t1, double t2)
{
	std::cout << "dc: " << t1 << "\n";
	if(t1 < 0 || t2 < 0 || t1 > t2 | t1 > timeWindow || t2 > timeWindow)
		return nullptr;

	double *filtered = fftw_alloc_real(samples*traces);
	std::vector<double> samplesForMean, means;
	size_t start, end;
	bool fs, fe;
	fs = fe = false;

	for(size_t j=0; j<samples; j++)
		if(timeDomain[j] >= t1 && !fs)
		{
			start = j;
			fs = true;
		}
		else if(timeDomain[j] > t2 && !fe)
		{
			end = j;
			fe = true;
			break;
		}

	for(size_t i=0; i<traces; i++)
	{
		for(size_t j=start; j<end; j++)
			samplesForMean.push_back(data[i*samples+j]);
		means.push_back(std::accumulate(samplesForMean.begin(), samplesForMean.end(), 0.0)/samplesForMean.size());
		samplesForMean.clear();
	}


	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			filtered[i*samples+j] = data[i*samples+j]-means[i];
	
	return filtered;
}


double* Profile::subtractDewow(double t1)
{
	std::cout << "dewow: " << t1 << "\n";
}
