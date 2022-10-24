#include "tb.hpp"

uint32_t Tinyblob::__cnt_blob = 0;
extern uint32_t blob_size, mode;

Tinyblob::Tinyblob(){
	posix_memalign(&__io_buffer, ALIGNMENT, blob_size);
	ti = (struct thread_info*)malloc(sizeof(struct thread_info));
	rbw = 0;
	__free = true;
}
                  
Tinyblob::~Tinyblob(){
	free(ti);
} 

void Tinyblob::setName(){
	__name = "device/blobs/B" + std::to_string(__cnt_blob++);
}

void Tinyblob::__open(){
	__index = __cnt_blob;
	setName();
	if (access(__name.c_str(), F_OK) == 0) {
		rbw = 1;
		__fd = open(__name.c_str(), O_RDWR|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
	} else {
		__fd = open(__name.c_str(), O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
	}
	__offset = 0;
}

void Tinyblob::__open(char *device){
	__index = __cnt_blob;	
	setName();
        if (access(device, F_OK) == 0) {     
		if(!mode)
			rbw = 1;
                __fd = open(device, O_RDWR|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
		created = 0;
        } else {                                                                                  
                __fd = open(device, O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
		created = 1;
        } 
}

void Tinyblob::__close(){
	close(__fd);
}
