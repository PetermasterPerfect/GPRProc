#include "profile.h"
#include <cmath>
#include <liquid.h>

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
	std::cout << "sampling freq: " << fs() << "\n";

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
	data = (float*) fftwf_malloc(samples*traces*sizeof(float));
	if(!data)
		throw std::runtime_error("Memory allocation error");
	for(size_t i=0; i<samples*traces; i++)
		data[i] = prof.data[i];

	readTimeDomain();
	if(prof.picks)
	{
		picks = new size_t[traces];
		for(size_t i=0; i<traces; i++)
			picks[i] = prof.picks[i];
	}

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

Profile::Profile(size_t tr, size_t samp, float tWin, float *buf) :
	traces(tr), samples(samp), timeWindow(tWin), data(buf)
{
	readTimeDomain();
	picks = naivePicking();
	init = true;
}


Profile::Profile(Profile *prof, float *buf)
{
	path = prof->path;
	samples = prof->samples;
	traces = prof->traces;
	timeWindow = prof->timeWindow;
	data = buf;

	timeDomain = new float[samples];
	for(unsigned i=0; i<samples; i++)
		timeDomain[i] = prof->timeDomain[i];

	marks = prof->marks;

	picks = naivePicking();
	init = true;
}

Profile::~Profile()
{
	if(data)
		fftwf_free(data);
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
		fftwf_plan p;
		fftwf_complex *fourier = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * traces*samples);
		float *trace = (float*) fftwf_malloc(sizeof(float)*samples);
		for (int i=0; i<samples; ++i)
			trace[i] = data[n*samples+i];

		p = fftwf_plan_dft_r2c_1d(samples, trace, fourier, FFTW_ESTIMATE);
		fftwf_execute(p);

		for (int i=0; i<samples/2; ++i)
		{
			x[i] = (double)i*fs()/samples;
			if(type == 1)
				y[i] = sqrt(pow(fourier[i][0], 2) + pow(fourier[i][1], 2));
			else
				y[i] = atan2(fourier[i][1], fourier[i][0]);
		}
		fftwf_destroy_plan(p);
		fftwf_free(fourier);
		fftwf_free(trace);
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

std::optional<std::pair<QCustomPlot*, QCPColorMap*>> Profile::createRadargram(QCPColorGradient::GradientPreset gradType, float scale)
{
	if(!init)
		return std::nullopt;

	MyQCustomPlot *imagePlot = new MyQCustomPlot(); // TODO: add parent

	imagePlot->xAxis = imagePlot->xAxis2;
	imagePlot->addGraph();
	imagePlot->xAxis2->setVisible(true);
	imagePlot->xAxis->setVisible(false);
	imagePlot->setInteraction(QCP::Interaction::iRangeZoom);
	imagePlot->setInteraction(QCP::Interaction::iRangeDrag);
	imagePlot->axisRect()->setRangeDragAxes(imagePlot->xAxis2, imagePlot->yAxis);
	imagePlot->axisRect()->setRangeZoomAxes(imagePlot->xAxis2, imagePlot->yAxis);

	imagePlot->xAxis2->setLabel("x");
	imagePlot->yAxis->setLabel("y");
	imagePlot->yAxis->setRangeReversed(true);
	float sampTime = timeWindow/samples;
	QCPRange *samplesRange = new QCPRange(0, timeWindow-sampTime ? timeWindow-sampTime : 1);
	QCPRange *tracesRange = new QCPRange(0,  traces-1 ? traces-1 : 1);
	QCPColorGradient gradient;
	gradient.loadPreset(gradType);
	QCPColorMapData *mapData = new QCPColorMapData(traces, samples, *tracesRange, *samplesRange);
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			mapData->setData(i, j*sampTime, data[i*samples+j]*scale);

	QCPColorMap *map = new QCPColorMap(imagePlot->xAxis2, imagePlot->yAxis);
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
		auto buf = new QRadioButton("Channel "+ QString::number(i+1), &dialog);
		options.push_back(buf);
		layout->addWidget(buf);
	}
	options[0]->setChecked(true);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
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
		data = read_typed_data<uint8_t>(in, sz, moveOff);
	else if(hdr.rh_bits == 16)
		data = read_typed_data<uint16_t>(in, sz, moveOff);
	else if(hdr.rh_bits == 32)
		data = read_typed_data<int32_t>(in, sz, moveOff);
	traces = sz/samples;


	readMarks(in, channel, offset, &hdr);
	if(!marks.size())
	{
		std::ifstream inDzx = open_both_cases(name, ".DZX"); 
		if(inDzx)
		{
			std::ostringstream oss;
			oss << inDzx.rdbuf(); 
			try 
			{
				marks = readMarksFromDzx(oss.str());
			}
			catch(const std::exception& e)
			{
				std::cerr << "Could not find marks in dzx file\n";
			}
		}
	}
	size_t zero;
	if(channel == 1)
		zero = hdr.rh_zero ? hdr.rh_zero : 2;
	else
		zero = hdr.rh_zero ? hdr.rh_zero : 0;

	if(zero >= samples)
		zero = 0;
	if(zero)
	{
		float *buf = (float*) fftwf_malloc(sizeof(float)*traces*(samples-zero));
		for(size_t i=0; i<traces; i++)
			for(size_t j=0; j<samples; j++)
				if(j>=zero)
					buf[i*(samples-zero)+(j-zero)] = data[i*samples+j];
		samples-=zero;

		fftwf_free(data);
		data = buf;
	}
	readTimeDomain();
}

std::vector<size_t> Profile::readMarksFromDzx(std::string source)
{
    std::vector<size_t> dzxmarks;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(source.c_str());
    if (!result)
    {
        throw std::runtime_error("Error while parsing xml");
    }

    pugi::xml_node root = doc.document_element();

    try
    {
        for (pugi::xml_node item : root.children("TargetGroup"))
        {
            for (pugi::xml_node child : item.children())
            {
                std::string tag(child.name());
                if (tag.find("TargetWayPt") != std::string::npos)
                {
                    for (pugi::xml_node gchild : child.children())
                    {
                        std::string gtag(gchild.name());
                        if (gtag.find("scanSampChanProp") != std::string::npos)
                        {
                            std::string text = gchild.text().as_string();
                            if (!text.empty())
                            {
                                size_t val;
                                sscanf(text.substr(0, text.find(',')).c_str(), "%zu", &val);
                                if (val < traces)
                                    dzxmarks.push_back(val);
                            }
                        }
                    }
                }
            }
        }
        if (dzxmarks.size() <= 1)
            throw std::runtime_error("not enough marks");
    }
    catch (const std::exception& e)
    {
        for (pugi::xml_node item : root.children("File"))
        {
            for (pugi::xml_node child : item.children())
            {
                std::string tag(child.name());
                if (tag.find("Profile") != std::string::npos)
                {
                    for (pugi::xml_node gchild : child.children())
                    {
                        std::string gtag(gchild.name());
                        if (gtag.find("WayPt") != std::string::npos)
                        {
                            for (pugi::xml_node ggchild : gchild.children())
                            {
                                std::string ggtag(ggchild.name());
                                if (ggtag.find("scan") != std::string::npos)
                                {
                                    size_t val = ggchild.text().as_ullong();
                                    if (val < traces)
                                        dzxmarks.push_back(val);
                                }
                            }
                        }
                    }
                }
            }
        }

        if (dzxmarks.size() < 1)
            throw std::runtime_error("not enough marks");
    }
    return dzxmarks;
}

void Profile::readMarks(std::ifstream &in, int channel, size_t offset, tagRFHeader *hdr)
{
	if(channel == 1)
	{
		detectMarks(data);
	}
	else
	{
		float *buf;
		size_t sz = samples*traces;
		in.seekg(offset-samples*(hdr->rh_bits/8), std::ios_base::beg);
		size_t moveOff = samples*(hdr->rh_nchan-1);
		if(hdr->rh_bits == 8)
		{
			buf = read_typed_data<uint8_t>(in, sz, moveOff);
			detectMarks(buf);
		}
		else if(hdr->rh_bits == 16)
		{
			buf = read_typed_data<uint16_t>(in, sz, moveOff);
			detectMarks(buf);
		}
		else if(hdr->rh_bits == 32)
		{
			buf = read_typed_data<int32_t>(in, sz, moveOff);

			for(size_t i=0; i<traces; i++)
				if(buf[i*samples+1] > 0)
					marks.push_back(i);
			if(marks.size() == traces) // all marks 
				marks.clear();
			fftwf_free(buf);
		}
	}
}

struct __attribute__((packed)) TraceValues {
    uint16_t normal_first;
    uint16_t normal_second;
    uint16_t marker_first;
    uint16_t marker_second;
};

void Profile::detectMarks(float *dt)
{
	std::pair<float, size_t> ty1, ty2;
	if(traces < 2)
		return;
	ty1.first = dt[1];
	ty2.first = dt[samples+1];
	size_t i = 2;
	while(ty1.first == ty2.first && i<traces)
	{
		ty2.first = dt[i*samples+1];
		i++;
	}

	if(ty1.first == ty2.first)
		return;

	for(size_t i=0; i<traces; i++)
		if(dt[i*samples+1] == ty1.first)
			ty1.second++;
		else if(dt[i*samples+1] == ty2.first)
			ty2.second++;

	float marker = ty1.second > ty2.second ? ty2.first : ty1.first;

	for(size_t i=0; i<traces; i++)
		if(dt[i*samples+1] == marker)
			marks.push_back(i);
	if(marks.size() == traces) // all marks 
		marks.clear();
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
		data = (float*) fftwf_malloc(sizeof(float)*sz);
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
			data = (float*) fftwf_malloc(sizeof(float)*sz);
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
	timeDomain = new float[samples];
	if(!timeDomain)
		throw std::runtime_error("No memory");

	float timeIt = timeWindow/samples;
	float curTime = 0;
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

float* Profile::maxSamplePerTrace()
{
	float *ret = new float[traces]{};
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			ret[i] =  fabs(data[i*samples+j]) > ret[i] ? fabs(data[i*samples+j]) : ret[i];
	return ret;
}

float Profile::maxAmplitude()
{
	float ret = data[0];
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			ret =  data[i*samples+j] > ret ? data[i*samples+j] : ret;
	return ret;
}

float Profile::minAmplitude()
{
	float ret = data[0];
	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			ret =  data[i*samples+j] < ret ? data[i*samples+j] : ret;
	return ret;
}

std::shared_ptr<Profile> Profile::subtractDcShift(float t1, float t2)
{
	std::cout << "dc: " << t1 << "\n";
	if(t1 < 0 || t2 < 0 || t1 > t2 | t1 > timeWindow || t2 > timeWindow)
		return std::shared_ptr<Profile>{};

	float *filtered = (float*) fftwf_malloc(sizeof(float)*samples*traces);
	std::vector<float> means;
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
		float mean = 0;
		for(size_t j=start; j<end; j++)
			mean += data[i*samples+j];
		means.push_back(mean/(end-start));
	}

	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
			filtered[i*samples+j] = data[i*samples+j]-means[i];
	
	return std::make_shared<Profile>(this, filtered);
}

std::shared_ptr<Profile> Profile::subtractDewow(float t1)
{
	if(t1 < 0 || t1 > timeWindow)
		return std::shared_ptr<Profile>{};
	if(t1 <= timeWindow/samples)
		return std::make_shared<Profile>(this, data);
	float *filtered = (float*) fftwf_malloc(sizeof(float)*samples*traces);
	size_t windowSize;
	size_t buf = lround(t1/(timeWindow/samples));
	windowSize = buf%2 ? buf : buf-1;
	if(windowSize == 1)
	{
		fftwf_free(filtered);
		return std::make_shared<Profile>(this, data);
	}

	std::vector<float> means;
	for(size_t i=0; i<traces; i++)
	{
		float mean = 0;
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
			float mean = 0;
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
			float mean = 0;
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

std::shared_ptr<Profile> Profile::gainFunction(float t1, float t2, float exponent, float maxVal)
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
	float *filtered = (float*) fftwf_malloc(sizeof(float)*samples*traces);

	for(size_t i=0; i<traces; i++)
		for(size_t j=startIdx; j<=endIdx; j++)
		{
			float t = timeDomain[j];
			filtered[i*samples+j] = exp((t-timeDomain[startIdx])*timeDomain[1]*exponent)*data[i*samples+j];
			
		}
	for(size_t i=0; i<traces; i++)
		for(size_t j=endIdx+1; j<samples; j++)
			filtered[i*samples+j] = exp((timeDomain[endIdx]-timeDomain[startIdx])*timeDomain[1]*exponent)*data[i*samples+j];
	
	const size_t sz = traces * samples;
	const float mean = std::accumulate(data, data + sz, 0.0) / sz;
	const float meanFilt = std::accumulate(filtered, filtered + sz, 0.0) / sz;

	float varianceData = std::accumulate(data, data + sz, 0.0,
		[&](float accumulator, const float& val) {
			return accumulator + (val - mean) * (val - mean);
		}) / (sz - 1);
	const float stdData = std::sqrt(varianceData);

	float varianceFilt = std::accumulate(filtered, filtered + sz, 0.0,
		[&](float accumulator, const float& val) {
			return accumulator + (val - meanFilt) * (val - meanFilt);
		}) / (sz - 1);
	float stdFilt = std::sqrt(varianceFilt);

	for(size_t i=0; i<traces; i++)
		for(size_t j=0; j<samples; j++)
		{
			filtered[i*samples+j] /= stdFilt;
			filtered[i*samples+j] *= stdData;
		}

	return std::make_shared<Profile>(this, filtered);
}



std::shared_ptr<Profile> Profile::ampltitudesTo0(float amplMin, float amplMax)
{
	if(amplMin > amplMax)
		return std::shared_ptr<Profile>{};

	float *filtered = (float*) fftwf_malloc(sizeof(float)*samples*traces);
	memcpy(filtered, data, traces*samples*sizeof(float));

	for(size_t i=0; i<traces*samples; i++)
		if(data[i] >= amplMin && data[i] <= amplMax)
			filtered[i] = 0;

	return std::make_shared<Profile>(this, filtered);
}


std::shared_ptr<Profile> Profile::xFlip()
{
	float *filtered = (float*) fftwf_malloc(sizeof(float)*samples*traces);

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
	float *filtered = (float*) fftwf_malloc(sizeof(float)*samples*traces);

	for(size_t i=0; i<traces; i++)
		for(size_t j=samples-1; j>=0; j--)
		{
			filtered[i*samples+(samples-1-j)] = data[i*samples+j];
			if(!j)
				break;
		}
	return std::make_shared<Profile>(this, filtered);
}

std::shared_ptr<Profile> Profile::timeCut(float t)
{
	if(t < 0)
		return std::shared_ptr<Profile>{};
	float sampTime = timeWindow/samples;

	float *filtered;
	size_t newSamples;
	if(t >= timeWindow)
	{
		newSamples = samples+static_cast<size_t>((t-timeWindow)/sampTime)+1;
		filtered = (float*) fftwf_malloc(sizeof(float)*newSamples*traces);
		for(size_t i=0; i<traces; i++)
			for(size_t j=0; j<newSamples; j++)
				filtered[i*newSamples+j] = j < samples ? data[i*samples+j] : 0;
	}
	else
	{
		newSamples = static_cast<size_t>(t/sampTime)+1;
		filtered = (float*) fftwf_malloc(sizeof(float)*newSamples*traces);
		for(size_t i=0; i<traces; i++)
			for(size_t j=0; j<samples; j++)
			{
				if(j >= newSamples)
					break;
				filtered[i*newSamples+j] = data[i*samples+j];
			}
	}
	std::cout << "time cut, samples" << newSamples << "\n";
	auto prof = std::make_shared<Profile>(traces, newSamples, sampTime*newSamples, filtered);
	if(marks.size())
		prof->marks = marks;
	return prof;
}


std::shared_ptr<Profile> Profile::moveStartTime(float t)
{
	if(t == 0)
		return std::make_shared<Profile>(*this);
	if(t > timeWindow)
		return std::shared_ptr<Profile>{};
	float sampTime = timeWindow/samples;
	float *filtered;
	size_t newSamples;
	if(t < 0)
	{
		newSamples = static_cast<size_t>(-1*t/sampTime)+1;
		filtered = (float*) fftwf_malloc(sizeof(float)*(samples-newSamples)*traces);
		for(size_t i=0; i<traces; i++)
			for(size_t j=newSamples; j<samples; j++)
				filtered[i*(samples-newSamples)+j-newSamples] = data[i*samples+j];
		newSamples = samples-newSamples;
	}
	else
	{
		newSamples = static_cast<size_t>(t/sampTime)+1;
		filtered = (float*) fftwf_malloc(sizeof(float)*(newSamples+samples)*traces);
		for(size_t i=0; i<traces; i++)
			for(size_t j=0; j<newSamples; j++)
				filtered[i*(newSamples+samples)+j] = 0;
		for(size_t i=0; i<traces; i++)
			for(size_t j=newSamples; j<newSamples+samples; j++)
				filtered[i*(newSamples+samples)+j] = data[i*samples+j-newSamples];
		newSamples+=samples;
	}
	std::cout << "time cut, samples" << newSamples << "\n";
	auto prof = std::make_shared<Profile>(traces, newSamples, sampTime*newSamples, filtered);
	if(marks.size())
		prof->marks = marks;
	return prof;

}


std::shared_ptr<Profile> Profile::butterworthFilter(float lowCut, float highCut, float att, float ripple)
{
	if(lowCut >= highCut)
		return std::shared_ptr<Profile>{};

	float *filtered = (float*) fftwf_malloc(sizeof(float)*samples*traces);
	float normLow = lowCut*1e+6/(fs());
	float normHigh = highCut*1e+6/(fs());
	float center = normLow+(normHigh-normLow)/2;

    unsigned int N = 4;
    float As = 60.0f;       // stopband attenuation [dB]
    float Ap = 1.0f;        // passband ripple [dB]

	for(size_t i=0; i<traces; i++)
	{
		iirfilt_rrrf filter = iirfilt_rrrf_create_prototype(LIQUID_IIRDES_BUTTER, LIQUID_IIRDES_BANDPASS, LIQUID_IIRDES_SOS,
				4, normLow, center, ripple, att);
		for(size_t j=0; j<samples; j++)
			iirfilt_rrrf_execute(filter, data[i*samples+j], &filtered[i*samples+j]);
		iirfilt_rrrf_destroy(filter);
	}

	return std::make_shared<Profile>(this, filtered);
}

size_t* Profile::naivePicking()
{
	const float threshold = 0.2;
	const float threshold1 = 0.05;
	float *maxs = maxSamplePerTrace();
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
