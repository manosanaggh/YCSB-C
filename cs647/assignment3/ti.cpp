#include "ti.hpp"

Tinyindex::Tinyindex(){
	tinfo = new Thread_info();
	allocated = false;
	fd = -1;
}

Tinyindex::~Tinyindex(){}
