//
//  cs647_db.cc
//  YCSB-C        
//                                                       
//  Created by Manos Anagnostakis on 10/23/22.           
//  Copyright (c) 2022 Manos Anagnostakis <manosanag@csd.uoc.gr>.
//  

#include "cs647_db.h"

extern pthread_barrier_t barrier;
extern Tinyindex *ti;
extern uint32_t mode, nprocs, blob_size;
extern long raw_size;
extern pthread_rwlock_t rwlock;
uint32_t allocate = 0;
extern Tinylog *tl;
extern void set_alarm();

namespace ycsbc {

void CS647DB::Init() {
	//pthread_t threads[1024];

        mode = 1;
        nprocs = 300;
        blob_size = 4096;
        raw_size = 10 * 1024L * 1024L * 1024L;

        /*if (pthread_rwlock_init(&rwlock, NULL) != 0) {
                printf("\n rwlock init has failed\n");
                return;
        }             
        pthread_barrier_init(&barrier, NULL, nprocs+1);*/

	if(!ti){
		tl = new Tinylog();
		ti = replay();

		if(mode){
        		if((tl->fd = open("/mnt/fmap/device/raw/wal.log", O_RDWR|O_CREAT|O_DIRECT|O_SYNC, S_IRUSR|S_IWUSR)) == -1){
                		std::cout << "[Init] Error with opening WAL" << std::endl;
				return;
			}
		}
		else{
                        if((tl->fd = open("/mnt/fmap/device/blobs/wal.log", O_RDWR|O_CREAT|O_DIRECT|O_SYNC, S_IRUSR|S_IWUSR)) == -1){
                                std::cout << "[Init] Error with opening WAL" << std::endl;
                                return;
                        }
		}
		
		int x;
		if((x = fallocate(tl->fd, 0, 0, raw_size)) == -1){
			std::cout << "[Init] WAL allocation error!" << std::endl;
			return;
		}

		if(!ti){
			ti = new Tinyindex();
		}
			if(mode)
				recover((char*)"/mnt/fmap/device/raw/pairs.txt");
			else
				recover((char*)"/mnt/fmap/device/blobs/pairs.txt");
		//}
        	
		for(uint32_t i = 0; i < nprocs; i++)      
                	//pthread_create(&threads[i], NULL, (void* (*)(void*))&tb_allocate_blob, NULL);
			tb_allocate_blob();
        	//pthread_barrier_wait(&barrier);
		allocate = 1;

		//set_alarm();
	}
}

void CS647DB::Close() {
	std::cout << "CS647DB::Close" << std::endl;
	if(allocate){
                if(mode)
                        persist((char*)"/mnt/fmap/device/raw/pairs.txt");
                else
                        persist((char*)"/mnt/fmap/device/blobs/pairs.txt");
		allocate = 0;
		truncate();
	}
        
	/*for(auto x : ti->__kv_store)
		for(auto y : x.second)
			if(y)
				std::cout << "Key: " << x.first << " | Value of blob " << y->index() << ": " << (char*)y->__io_buffer << std::endl;*/
	//pthread_barrier_destroy(&barrier);
        //pthread_rwlock_destroy(&rwlock);
	std::cout << "Closing db" << std::endl;
}


int CS647DB::Read(const std::string &table, const std::string &key,
        const std::vector<std::string> *fields,
        std::vector<KVPair> &result) {

	std::vector<Tinyblob*> *pv = NULL;
	//std::cout << "[READ] going on..." << std::endl;
        if((pv = get(key)) != NULL){
                for(auto x : *pv)
			result.push_back(std::make_pair(key, std::string((char*)x->__io_buffer)));
		//return 0;
	}
	//std::cout << "[READ] returning..." << std::endl;
	return 0;
}

int CS647DB::Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result){
	//if(ti->__kv_store[key].empty())
	//	return 0;

	std::vector<KVPair> result2;
	static uint32_t x = 0;
	ti->scanner = scan_init();
        if(!ti->scanner.is_open()) {                                                             
		std::cout << "[SCAN] Error scanner not open!" << std::endl;      
		return -1;
        }  

	std::cout << "[SCAN] going on..." << std::endl;
	std::cout << x++ << std::endl;
	for(int i = 0; i < len; i++){
		for(auto x : *(get_scan_value(ti->next_pair)))
			result2.push_back(std::make_pair(get_scan_key(ti->next_pair), std::string((char*)x->__io_buffer)));
		result.push_back(result2);
		if(get_next(&ti->scanner) == -1)
			return 0;
	}
	return close_scanner(&ti->scanner);
}

int CS647DB::Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){
        if(ti->__kv_store[key].empty())
                return 0;
	//std::cout << "[UPDATE] going on..." << std::endl;
        //pthread_t threads[1024];

        //pthread_barrier_destroy(&barrier);
        //pthread_barrier_init(&barrier, NULL, 2);

        for(int i = 0; i < (int)values.size(); i++){
                Thread_info *tinfo = new Thread_info();                 
                tinfo->key = key;                              
                tinfo->value = values[i].second;                  
                //pthread_create(&threads[i], NULL, (void* (*)(void*))&put, tinfo);
                //pthread_barrier_wait(&barrier);
		put(tinfo);
                /*if(tinfo->result == -1)
                        std::cout << "[UPDATE] No free blobs!" << std::endl;
		else
			std::cout << "[UPDATE] Successful!" << std::endl;*/
        }
        return 0;
}

int CS647DB::Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){
	if(!ti->__kv_store[key].empty())
		return -1;
	//pthread_t threads[1024];

        //pthread_barrier_destroy(&barrier);
        //pthread_barrier_init(&barrier, NULL, 2);

	for(int i = 0; i < (int)values.size(); i++){
                Thread_info *tinfo = new Thread_info();                 
                tinfo->key = key;                              
                tinfo->value = values[i].second;                  
                //pthread_create(&threads[i], NULL, (void* (*)(void*))&put, tinfo);
                //pthread_barrier_wait(&barrier);
		//std::cout << "Value: " << tinfo->value << std::endl;
		put(tinfo);
                /*if(tinfo->result == -1)
                        std::cout << "[INSERT] No free blobs!" << std::endl;
		else
			std::cout << "[INSERT] Successful!" << std::endl;
			*/
	}
	return 0;
}       

int CS647DB::Delete(const std::string &table, const std::string &key){
        Thread_info *tinfo = new Thread_info();                                               
        tinfo->key = key;

	if(erase(tinfo) == -1){
		return -1;
	}
	std::cout << "[DELETE] going on..." << std::endl;
	return 0;
}

} // namespace ycsbc


