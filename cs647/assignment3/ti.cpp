#include "ti.hpp"

Tinyindex::Tinyindex(){
	tinfo = new Thread_info();
	allocated = false;
}

Tinyindex::~Tinyindex(){}
