#include "profile.h"
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
		open_ss(name);
		//load dt1
	else
		std::cout << "Unsupport extension\n";
	picks = naivePicking();
std::cout << "Samples: " << samples << " , traces: " << traces << ", timewindow: " << timeWindow << ", sample time: " << timeWindow/samples << "\n"; 

	for(size_t i=0; i<marks.size(); i++)
		std::cout << marks[i] << " ";
	std::cout << "\n";
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

	picks = new size_t[samples];
	for(unsigned i=0; i<samples; i++)
		picks[i] = prof.picks[i];

	marks = prof.marks;


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
	picks = prof.picks;
	prof.picks = nullptr;

	marks = prof.marks;

	init = true;
}

Profile::Profile(Profile *prof, double *buf)
{
	path = prof->path;
	samples = prof->samples;
	traces = prof->traces;
	timeWindow = prof->timeWindow;
	data = buf;

	timeDomain = new double[samples];
	for(unsigned i=0; i<samples; i++)
		timeDomain[i] = prof->timeDomain[i];

	marks = prof->marks;

	picks = naivePicking();
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

	if(picks)
		delete[] picks;
	picks = nullptr;
}

std::pair<QVector<double>, QVector<double>> Profile::prepareWiggleData(size_t n, char type)
{
	QVector<double> x(samples), y(samples);
	switch(type)
	{
	case 0: // trace
		for (int i=0; i<samples; ++i)
		{
			x[i] = i;
			y[i] = data[n*samples+i];
		}
		break;
	case 1: // amplitude
	case 2:
		fftw_plan p;
		fftw_complex *fourier = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * traces*samples);
		double *trace = fftw_alloc_real(samples);
		for (int i=0; i<samples; ++i)
			trace[i] = data[n*samples+i];

		p = fftw_plan_dft_r2c_1d(samples, trace, fourier, FFTW_ESTIMATE);
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

	return std::make_pair(x, y);
}

QCustomPlot* Profile::createWiggle(size_t n, char type)
{
	if(!init)
		return nullptr;

	QCustomPlot *wigglePlot = new QCustomPlot();
	auto wiggleData = prepareWiggleData(n, type);

	wigglePlot->addGraph();
	wigglePlot->graph(0)->setData(wiggleData.first, wiggleData.second);
	wigglePlot->xAxis->setLabel("x");
	wigglePlot->yAxis->setLabel("y");

	if(type == 0)
	{
		QCPGraph *marker = wigglePlot->addGraph();
		marker->addData(picks[n], data[n*samples+picks[n]]);
		marker->setLineStyle(QCPGraph::lsNone);
		marker->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 10));
		marker->setPen(QPen(Qt::red));

		QCPScatterStyle myScatter;
		myScatter.setShape(QCPScatterStyle::ssCircle);
		myScatter.setPen(QPen(Qt::blue));
		myScatter.setBrush(Qt::white);
		myScatter.setSize(5);
		wigglePlot->graph(0)->setScatterStyle(myScatter);
		wigglePlot->graph(1)->rescaleAxes(true);
	}
	wigglePlot->graph(0)->rescaleAxes();
	wigglePlot->replot();
	return wigglePlot;
}

std::optional<std::pair<QCustomPlot*, QCPColorMap*>> Profile::createRadargram(QCPColorGradient::GradientPreset gradType, double scale)
{
	if(!init)
		return std::nullopt;

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
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			mapData->setData(i, j, data[i*samples+j]*scale);

	QCPColorMap *map = new QCPColorMap(imagePlot->xAxis, imagePlot->yAxis);
	map->setData(mapData);
	map->setGradient(gradient);
	map->rescaleDataRange();
	map->setInterpolate(false);
	map->setTightBoundary(false);
	imagePlot->rescaleAxes();
	imagePlot->replot();
	delete samplesRange;
	delete tracesRange;
	return std::make_optional(std::make_pair(imagePlot, map));;
}


int Profile::askForChannelDialog(tagRFHeader *hdr)
{
	QDialog dialog;
    dialog.setWindowTitle("Choose a channel");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
	std::vector<QRadioButton*> options;
	for(int i=0; i<hdr->rh_nchan; i++)
	{
		auto buf = new QRadioButton("Channel "+ QString::number(i+1));
		options.push_back(buf);
		layout->addWidget(buf);
	}
	options[0]->setChecked(true);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) 
	{
		for(int i=0; i<hdr->rh_nchan; i++)
			if(options[i]->isChecked())
				return i+1;
    } 
	else
		return -1;
}


void Profile::open_gssi(std::string name)
{
	std::ifstream in = open_both_cases(name, ".DZT"); 
	if(!in)
		throw std::invalid_argument("No dzt file");

	tagRFHeader hdr;
	in.read(reinterpret_cast<char*>(&hdr), sizeof(tagRFHeader));
	int channel = hdr.rh_nchan > 1 ? askForChannelDialog(&hdr) : 1;

	samples = hdr.rh_nsamp;
	timeWindow = hdr.rhf_range;

	in.seekg(0, std::ios_base::end);
	size_t offset;
	if(hdr.rh_data < MINHEADERSIZE)
		offset = hdr.rh_data*MINHEADERSIZE;
	else
		offset = hdr.rh_nchan*MINHEADERSIZE;

	size_t sz = static_cast<size_t>(in.tellg());
	if(sz <= offset)
		throw std::invalid_argument("Bad offset");
	sz -= offset;
	sz /= hdr.rh_bits/8;
	sz /= hdr.rh_nchan;
	size_t moveOff;
	if(hdr.rh_nchan > 1)
	{
		if(channel == 1)
			in.seekg(offset-samples*(hdr.rh_bits/8), std::ios_base::beg);
		else
			in.seekg(offset, std::ios_base::beg);
		moveOff = samples*(hdr.rh_nchan-1);
	}
	else
	{
		in.seekg(offset, std::ios_base::beg);
		moveOff = 0;
	}
	if(hdr.rh_bits == 8)
		read_typed_data<uint8_t>(in, sz, moveOff);
	else if(hdr.rh_bits == 16)
		read_typed_data<uint16_t>(in, sz, moveOff);
	else if(hdr.rh_bits == 32)
		read_typed_data<int32_t>(in, sz, moveOff);
	traces = sz/samples;

	std::ifstream inDzx = open_both_cases(name, ".DZX"); 
	if(inDzx)
	{
		std::cerr << "NOT IMPLEMENTED\n";
	}
	else
	{
		double max = 0;
		for(size_t i=0; i<traces; i++)
			if(data[i*samples+1] > 0)
				marks.push_back(i);
		if(marks.size() == traces) // all marks 
			marks.clear();
		
		size_t zero = hdr.rh_zero ? hdr.rh_zero : 2;
		double *buf = fftw_alloc_real(traces*(samples-zero));
		for(size_t i=0; i<traces; i++)
			for(size_t j=0; j<samples; j++)
				if(j>=zero)
					buf[i*(samples-zero)+(j-zero)] = data[i*samples+j];
		samples-=zero;

		fftw_free(data);
		data = buf;
	}

	readTimeDomain();
}


void Profile::open_ss(std::string name)
{
	bool isHdr = true;
	try
	{
		read_hd(name);
	}
	catch(std::invalid_argument)
	{
		isHdr = false;
	}
	std::ifstream in = open_both_cases(name, ".DT1"); 
	if(!in)
		throw std::invalid_argument("No dzt file");

	if(isHdr)
	{
		size_t sz = samples*traces;
		data = fftw_alloc_real(sz);
		if(!data)
			throw std::runtime_error("No memory");
		for(int i=0; i<sz; i++)
		{
			if(i%samples == 0)
				in.seekg(sizeof(SsTraceHdrStruct), std::ios_base::cur);
			int16_t buf;
			in.read(reinterpret_cast<char*>(&buf), sizeof(int16_t));
			data[i] = buf;
		}
	}
	else
	{
		traces = 0;

		float buf;
		in.seekg(8, std::ios_base::cur);
		in.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		samples = buf;
		in.seekg(20, std::ios_base::cur);
		in.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		timeWindow = buf ? buf : 400;
		std::cout << "tw: " << timeWindow << "\n";
		in.seekg(0, std::ios_base::end);
		size_t fileSz = in.tellg();
		if(fileSz % (sizeof(SsTraceHdrStruct)+samples*sizeof(int16_t)) == 0)
		{
			in.seekg(0, std::ios_base::beg);
			traces = fileSz / (sizeof(SsTraceHdrStruct)+samples*sizeof(int16_t));
			size_t sz = samples*traces;
			data = fftw_alloc_real(sz);
			if(!data)
				throw std::runtime_error("No memory");
			for(int i=0; i<sz; i++)
			{
				if(i%samples == 0)
					in.seekg(sizeof(SsTraceHdrStruct), std::ios_base::cur);
				int16_t buf;
				in.read(reinterpret_cast<char*>(&buf), sizeof(int16_t));
				data[i] = buf;
			}
		}
	}

	readTimeDomain();

}

void Profile::read_hd(std::string name)
{
	std::ifstream in = open_both_cases(name, ".HD"); 
	if(!in)
		throw std::invalid_argument("No hd header file");
	std::string line;
	while(std::getline(in, line)) 
	{
		size_t sep = line.find("=");
		if(sep == std::string::npos)
			continue;
		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep+1);
		boost::algorithm::trim(key);
		boost::algorithm::trim(value);
		if(key == "NUMBER OF PTS/TRC")
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
		else if(key == "TOTAL TIME WINDOW")
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

void Profile::open_mala(std::string name, bool f)
{
	read_rad(name);
	if(!f)
		read_rd37<short>();
	else
		read_rd37<int>();

	readTimeDomain();
}


void Profile::readTimeDomain()
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
		size_t sep = line.find(":");
		if(sep == std::string::npos)
			continue;

		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep+1);
		boost::algorithm::trim(key);
		boost::algorithm::trim(value);

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
			timeWindow = std::stod(value)*1000;
			if(timeWindow <= 0)
			{
				in.close();
				throw std::invalid_argument("Bad header info");
			}
		}
	}
	in.close();
}

double* Profile::maxSamplePerTrace()
{
	double *ret = new double[traces]{};
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			ret[i] =  fabs(data[i*samples+j]) > ret[i] ? fabs(data[i*samples+j]) : ret[i];
	return ret;
}

double Profile::maxAmplitude()
{
	double ret = data[0];
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			ret =  data[i*samples+j] > ret ? data[i*samples+j] : ret;
	return ret;
}

double Profile::minAmplitude()
{
	double ret = data[0];
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			ret =  data[i*samples+j] < ret ? data[i*samples+j] : ret;
	return ret;
}

std::shared_ptr<Profile> Profile::subtractDcShift(double t1, double t2)
{
	std::cout << "dc: " << t1 << "\n";
	if(t1 < 0 || t2 < 0 || t1 > t2 | t1 > timeWindow || t2 > timeWindow)
		return std::shared_ptr<Profile>{};

	double *filtered = fftw_alloc_real(samples*traces);
	std::vector<double> means;
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
		double mean = 0;
		for(size_t j=start; j<end; j++)
			mean += data[i*samples+j];
		means.push_back(mean/(end-start));
	}

	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			filtered[i*samples+j] = data[i*samples+j]-means[i];
	
	return std::make_shared<Profile>(this, filtered);
}

std::shared_ptr<Profile> Profile::subtractDewow(double t1)
{
	if(t1 < 0 || t1 > timeWindow)
		return std::shared_ptr<Profile>{};
	if(t1 <= timeWindow/samples)
		return std::make_shared<Profile>(this, data);
	double *filtered = fftw_alloc_real(samples*traces);
	size_t windowSize;
	size_t buf = lround(t1/(timeWindow/samples));
	windowSize = buf%2 ? buf : buf-1;
	if(windowSize == 1)
	{
		fftw_free(filtered);
		return std::make_shared<Profile>(this, data);
	}

	std::vector<double> means;
	for(size_t i=0; i<traces; i++)
	{
		double mean = 0;
		for(size_t j=0; j<windowSize/2; j++)
			mean += data[i*samples+j];
		mean /= windowSize/2;
		means.push_back(mean);
	}
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<windowSize/2; j++)
			filtered[i*samples+j] = data[i*samples+j]-means[i];

	means.clear();
	for(size_t i=0; i<traces; i++)
		{
			double mean = 0;
			for(size_t j=samples-windowSize/2; j<samples; j++)
				mean += data[i*samples+j];
			mean /= windowSize/2;
			means.push_back(mean);
		}
	for(size_t i=0; i<traces; i++)
		for(size_t j=samples-windowSize/2; j<samples; j++)
			filtered[i*samples+j] = data[i*samples+j]-means[i];

	for(size_t i=0; i<traces; i++)
	{
		for(size_t j=windowSize/2; j<samples-windowSize/2; j++)
		{
			double mean = 0;
			size_t start = j-windowSize/2;
			size_t end = j+windowSize/2;
			for(size_t k=start; k<end; k++)
				mean += data[i*samples+k];
			mean /= end-start;
			filtered[i*samples+j] = data[i*samples+j]-mean;
		}
	}

	return std::make_shared<Profile>(this, filtered);
}

std::shared_ptr<Profile> Profile::gainFunction(double t1, double t2, double exponent, double maxVal)
{
	if(t1 < 0 || t2 < 0 || t1 > t2 | t1 > timeWindow || t2 > timeWindow)
		return std::shared_ptr<Profile>{};
	size_t startIdx = 0, endIdx = samples-1;
	bool f = 0;
	for(size_t i=0; i<samples; i++)
	{
		if(timeDomain[i] >= t1 && !f)
		{
			startIdx = i;
			f = 1;
		}
		if(timeDomain[i] >= t2)
		{
			endIdx = i;
			break;
		}
	}
	std::cout << startIdx << ", " << endIdx << " <--\n";
	double *filtered = fftw_alloc_real(samples*traces);

	for(size_t i=0; i<traces; i++)
		for(size_t j=startIdx; j<=endIdx; j++)
		{
			double t = timeDomain[j];
			filtered[i*samples+j] = exp((t-timeDomain[startIdx])*timeDomain[1]*exponent)*data[i*samples+j];
			
		}
	for(size_t i=0; i<traces; i++)
		for(size_t j=endIdx+1; j<samples; j++)
			filtered[i*samples+j] = exp((timeDomain[endIdx]-timeDomain[startIdx])*timeDomain[1]*exponent)*data[i*samples+j];
	
	const size_t sz = traces * samples;
	const double mean = std::accumulate(data, data + sz, 0.0) / sz;
	const double meanFilt = std::accumulate(filtered, filtered + sz, 0.0) / sz;

	double varianceData = std::accumulate(data, data + sz, 0.0,
		[&](double accumulator, const double& val) {
			return accumulator + (val - mean) * (val - mean);
		}) / (sz - 1);
	const double stdData = std::sqrt(varianceData);

	double varianceFilt = std::accumulate(filtered, filtered + sz, 0.0,
		[&](double accumulator, const double& val) {
			return accumulator + (val - meanFilt) * (val - meanFilt);
		}) / (sz - 1);
	double stdFilt = std::sqrt(varianceFilt);

	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
		{
			filtered[i*samples+j] /= stdFilt;
			filtered[i*samples+j] *= stdData;
		}

	return std::make_shared<Profile>(this, filtered);
}



std::shared_ptr<Profile> Profile::ampltitudesTo0(double amplMin, double amplMax)
{
	if(amplMin > amplMax)
		return std::shared_ptr<Profile>{};

	double *filtered = fftw_alloc_real(samples*traces);
	memcpy(filtered, data, traces*samples*sizeof(double));

	for(size_t i=0; i<traces*samples; i++)
		if(data[i] >= amplMin && data[i] <= amplMax)
			filtered[i] = 0;

	return std::make_shared<Profile>(this, filtered);
}


std::shared_ptr<Profile> Profile::xFlip()
{
	double *filtered = fftw_alloc_real(samples*traces);

	for(size_t i=traces-1; i>=0; i--)
	{
		for(size_t j=0; j<samples; j++)
			filtered[(traces-1-i)*samples+j] = data[(i)*samples+j];
		if(!i)
			break;
	}
	return std::make_shared<Profile>(this, filtered);
}

std::shared_ptr<Profile> Profile::yFlip()
{
	double *filtered = fftw_alloc_real(samples*traces);

	for(size_t i=0; i<traces; i++)
		for(size_t j=samples-1; j>=0; j--)
		{
			filtered[i*samples+(samples-1-j)] = data[i*samples+j];
			if(!j)
				break;
		}
	return std::make_shared<Profile>(this, filtered);
}

size_t* Profile::naivePicking()
{
	const double threshold = 0.2;
	const double threshold1 = 0.05;
	double *maxs = maxSamplePerTrace();
	size_t *picks = new size_t[traces]{};
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			if(fabs(data[i*samples+j]) >= fabs(threshold*maxs[i]))
			{
				size_t k = j;
				while(k>=0)
				{
					if(fabs(data[i*samples+k]) <= fabs(threshold1*maxs[i]))
					{
						picks[i] = k;
						break;
					}
					if(k==0)
						break;
					k--;
				}
				break;
			}
	delete[] maxs;
	return picks;
}
