#include "tl.hpp"
          
Tinylog::Tinylog(){
        fd = -1;
	offset = 0;
	wal_buf = "";
	buf_sz = 0;
}                  
                 
Tinylog::~Tinylog(){
	close(fd);
}
