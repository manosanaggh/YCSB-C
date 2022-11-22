#include "tb.hpp"

uint32_t Tinyblob::__cnt_blob = 0;
int Tinyblob::raw_fd = -1;
std::string Tinyblob::DEVICE_PATH_PREFIX(DEV_PATH_PREF);
extern uint32_t blob_size;

Tinyblob::Tinyblob(){
	posix_memalign(&__io_buffer, ALIGNMENT, blob_size);
	ti = (struct thread_info*)malloc(sizeof(struct thread_info));
	__free = true;
	__persisted = false;
	__fd = -1;
}
                  
Tinyblob::~Tinyblob(){
	free(ti);
} 

void Tinyblob::setName(){
	__name = "/mnt/fmap/device/blobs/B" + std::to_string(__cnt_blob++);
}

void Tinyblob::__open(){
	__index = __cnt_blob;
	setName();
	if (access(__name.c_str(), F_OK) == 0) {
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
                if(raw_fd == -1)
			raw_fd = open(device, O_RDWR|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
		created = 0;
        } else {                                                                                  
                if(raw_fd == -1)
			raw_fd = open(device, O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
		created = 1;
        } 
}

void Tinyblob::__close(){
	close(__fd);
}
