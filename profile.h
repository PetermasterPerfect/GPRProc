#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <utility>
#include <map>

using file_pair = std::pair<std::string, std::string>;// file_pair;
std::ifstream open_both_cases(std::string name, std::string ext);
file_pair split_filename(std::string fname);

struct Profile
{
	std::string path;
	uint32_t samples;
	uint32_t lastTrace;
	std::unique_ptr<double[]> data;
	Profile(std::string);

private:
	void open_mala(std::string, bool f=0);
	void read_rad(std::string);
	template<class T>
	void read_rd37()
	{
		std::ifstream in(path, std::ios::binary); 
		if(!in)
			throw std::invalid_argument("No rd37 binary file");

		in.seekg(0, std::ios_base::end);
		size_t sz = static_cast<size_t>(in.tellg());
		in.seekg(0, std::ios_base::beg);

		data = std::make_unique<double[]>(sz/sizeof(T));
		for(int i=0; i<sz/sizeof(T); i++)
		{
			T buf;
			in.read(reinterpret_cast<char*>(&buf), sizeof(T));
			data[i] = buf;
		}

	}
	
};
