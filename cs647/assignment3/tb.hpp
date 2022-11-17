#ifndef TB
#define TB

#include <stdint.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include <dirent.h>
#include <bits/stdc++.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <cstdio>

#define ALIGNMENT 512

struct thread_info{                                                                                                                         
        int i;                                                                                 
        void *buffer; 
	int result;
	bool parallel;
};   

class Tinyblob{
	private:
		uint32_t __index;
		int __fd;
		std::string __name;
		static uint32_t __cnt_blob;
		uint32_t __offset;

	public:
		Tinyblob();

		~Tinyblob();

		void *__io_buffer;	
		uint32_t created;
		struct thread_info *ti;
		bool __free;
		bool __persisted;
		static int raw_fd;

		inline void setIndex(uint32_t index){
        		__index = index;
		}

		inline uint32_t index(){
        		return __index;
		}

		inline void setFd(int fd){
			__fd = fd;
		}

		inline int fd(){
			return __fd;
		}

		inline void printTb(){
			/*
			std::cout << "INDEX: " << __index 
				<< "\tFD: " << __fd
				<< "\tRAW_FD: " << raw_fd
				<< "\tNAME: " << __name 
				<< "\tBLOB_COUNT: " << __cnt_blob 
				<< "\tIO_BUFFER: " << (char*)__io_buffer
				<< "\tRAW DEVICE OFFSET: " << __offset
				<< std::endl;*/
		}

		void __open();

		void __open(char *device);

		void __close();

		static inline void setCntBlob(uint32_t cnt_blob){
			__cnt_blob = cnt_blob;
		}

		static inline uint32_t cnt_blob(){
			return __cnt_blob;
		}

		void setName();

		inline void setName(std::string name){
			__name.assign(name);
		};

		inline std::string name(){
			return __name;	
		}

		inline void setOffset(uint32_t offset){
			__offset = offset;
		}

		inline uint32_t offset(){
			return __offset;
		}

		inline bool is_free(){
			return __free;
		}
};

#endif
