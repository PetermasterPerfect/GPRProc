#include "profile.h"

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
		std::cout << "ok;";
		//load dzt
	else if(boost::algorithm::to_lower_copy(ext) == "dt1")
		std::cout << "ok;";
		//load dt1
	else
		std::cout << "Unsupport extension\n";
}

void Profile::open_mala(std::string name, bool f)
{
	read_rad(name);
	if(!f)
		read_rd37<short>();
	else
		read_rd37<int>();
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
				throw std::invalid_argument("Bad header info");
		}
		else if(key == "LAST TRACE")
		{
			lastTrace = std::stol(value);
			if(lastTrace <= 0)
				throw std::invalid_argument("Bad header info");
		}
	}
}

