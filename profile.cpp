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

Profile::Profile(const Profile& prof)
{
	if(!prof.init)
		return;
	path = prof.path;
	samples = prof.samples;
	lastTrace = prof.lastTrace;
	data = fftw_alloc_real(samples*lastTrace);
	if(!data)
		throw std::runtime_error("Memory allocation error");
	for(size_t i=0; i<samples*lastTrace; i++)
		data[i] = prof.data[i];
	init = true;
}


Profile::~Profile()
{
	if(data)
		fftw_free(data);
	data = nullptr;
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
		fftw_complex *fourier = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * lastTrace*samples);
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
				y[i] = atan(fourier[i][1]/fourier[i][0]);
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

QCustomPlot* Profile::createGraph()
{
	if(!init)
		return nullptr;

	QCustomPlot *imagePlot = new QCustomPlot();

	imagePlot->addGraph();
	imagePlot->xAxis->setLabel("x");
	imagePlot->yAxis->setLabel("y");
	imagePlot->yAxis->setRangeReversed(true);
	QCPRange *samplesRange = new QCPRange(1, samples);
	QCPRange *tracesRange = new QCPRange(1, lastTrace);
	QCPColorMapData *mapData = new QCPColorMapData(lastTrace, samples, *tracesRange, *samplesRange);
	for(size_t i=1; i<=lastTrace; i++)
		for(size_t j=1; j<=samples; j++)
			mapData->setData(i, j, data[(i-1)*samples+(j-1)]);
	QCPColorMap *map = new QCPColorMap(imagePlot->xAxis, imagePlot->yAxis);
	map->setData(mapData);
	map->rescaleDataRange();
	imagePlot->rescaleAxes();
	imagePlot->replot();
	delete samplesRange;
	delete tracesRange;
	return imagePlot;
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

	in.seekg(0, std::ios_base::end);
	size_t offset = sizeof(DztHdrStruct)*hdr.rh_nchan;
	size_t sz = static_cast<size_t>(in.tellg());
	if(sz <= offset)
		throw std::invalid_argument("Bad offset");
	sz -= offset;
	lastTrace = sz/(hdr.rh_bits/8)/samples;
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
			lastTrace = std::stol(value);
			if(lastTrace <= 0)
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
	std::cout << "Samples: " << samples << " , traces: " << lastTrace << "\n"; 
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
			lastTrace = std::stol(value);
			if(lastTrace <= 0)
			{
				in.close();
				throw std::invalid_argument("Bad header info");
			}
		}
	}
	in.close();
}
