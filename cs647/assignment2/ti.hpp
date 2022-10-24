#ifndef TI
#define TI

#include <iostream>
#include <unordered_map>
#include <iterator>
#include <vector>
#include "tb.hpp"
#include <boost/algorithm/string.hpp>

#define ALIGNMENT 512
#define MAX_BLOBS 32

class Thread_info{
	private:
	public:
		std::vector<Tinyblob*> pv;
		std::string key;
		std::string value;
        	int result;   
};

class Tinyindex{
	private:
	public:
		Tinyindex();
		~Tinyindex();
		std::unordered_map<std::string, std::vector<Tinyblob*>> __kv_store;
		std::string next_pair;
		std::ifstream scanner;
		Thread_info *tinfo;
};

#endif
