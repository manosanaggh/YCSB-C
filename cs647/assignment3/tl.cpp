#include "tl.hpp"
          
Tinylog::Tinylog(){
        fd = -1;
	offset = 0;
}                  
                 
Tinylog::~Tinylog(){
	close(fd);
}
