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
extern uint32_t mode, blob_size;
extern long raw_size;
extern pthread_rwlock_t rwlock;
extern std::vector<Tinyblob*>blobs;
uint32_t allocate = 0;
extern Tinylog *tl;
extern void set_alarm();

namespace ycsbc {

void CS647DB::Init() {
        mode = MODE;
        blob_size = BLOB_SIZE;
        raw_size = RAW_SIZE;

        if (pthread_rwlock_init(&rwlock, NULL) != 0) {
                printf("\n rwlock init has failed\n");
                return;
        }             
	pthread_barrier_init(&barrier, NULL, 1);
	pthread_rwlock_wrlock(&rwlock);
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
		if(!tl->fd && (x = fallocate(tl->fd, 0, 0, raw_size)) == -1){
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
        	
		allocate = 1;

		for(int i = 0; i < 60000; i++)
			tb_allocate_blob();

		//set_alarm();
	}
	pthread_rwlock_unlock(&rwlock);
	pthread_barrier_wait(&barrier);
}

void CS647DB::Close() {
	        pthread_rwlock_wrlock(&rwlock);
	std::cout << "CS647DB::Close" << std::endl;
	if(allocate){
                if(mode)
                        persist((char*)"/mnt/fmap/device/raw/pairs.txt");
                else
                        persist((char*)"/mnt/fmap/device/blobs/pairs.txt");
		allocate = 0;
		truncate();
	}
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
	pthread_barrier_destroy(&barrier);
        pthread_rwlock_destroy(&rwlock);
	std::cout << "Closing db" << std::endl;
}


int CS647DB::Read(const std::string &table, const std::string &key,
        const std::vector<std::string> *fields,
        std::vector<KVPair> &result) {
        pthread_rwlock_wrlock(&rwlock);
	std::vector<Tinyblob*> *pv = NULL;
        if((pv = get(key)) != NULL){
                for(auto x : *pv)
			result.push_back(std::make_pair(key, std::string((char*)x->__io_buffer)));
	}
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
	return 0;
}

int CS647DB::Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result){
	        pthread_rwlock_wrlock(&rwlock);
	if(ti->__kv_store[key].empty()){
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
		return 1;
	}

	std::vector<KVPair> result2;
	ti->scanner = scan_init();
        if(!ti->scanner.is_open()) {                                                             
		std::cout << "[SCAN] Error scanner not open!" << std::endl;      
		return 1;
        }  

	//std::cout << "[SCAN] going on..." << std::endl;
	for(int i = 0; i < len; i++){
		for(auto x : *(get_scan_value(ti->next_pair)))
			result2.push_back(std::make_pair(get_scan_key(ti->next_pair), std::string((char*)x->__io_buffer)));
		result.push_back(result2);
		if(get_next(&ti->scanner) == -1){
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
			std::cout << "return" << std::endl;
			return 1;
		}
		//std::cout << i << std::endl;
	}
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
	return close_scanner(&ti->scanner);
}

int CS647DB::Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){
	        pthread_rwlock_wrlock(&rwlock);
        if(ti->__kv_store[key].empty()){
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
                return 1;
	}

        Thread_info *tinfo = new Thread_info();                 
        tinfo->key = key;   

        for(int i = 0; i < (int)values.size(); i++){
		tinfo->value += values[i].first + ":" + values[i].second + "\n";
        }
	strcpy((char*)(ti->__kv_store[tinfo->key][0])->__io_buffer, tinfo->value.c_str());
	ti->__kv_store[tinfo->key][0]->__persisted = false;
        //if(tinfo->result == -1)
          //      std::cout << "[UPDATE] No free blobs!" << std::endl;
        //else
  //              std::cout << "[UPDATE] Successful!" << std::endl;
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
        return 0;
}

int CS647DB::Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){
	        pthread_rwlock_wrlock(&rwlock);
	if(!ti->__kv_store[key].empty()){
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
		return 1;
	}
	Thread_info *tinfo = new Thread_info();
	tinfo->key = key;
	for(int i = 0; i < (int)values.size(); i++){
                tinfo->value += values[i].first + ":" + values[i].second + "\n";                  
	}
        put(tinfo);
	/*if(tinfo->result == -1)
		std::cout << "[INSERT] No free blobs!" << std::endl;
	else
		std::cout << "[INSERT] Successful!" << std::endl;
*/
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
	return 0;
}       

int CS647DB::Delete(const std::string &table, const std::string &key){
	        pthread_rwlock_wrlock(&rwlock);
        Thread_info *tinfo = new Thread_info();                                               
        tinfo->key = key;

	if(erase(tinfo) == -1){
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
		return 1;
	}
	std::cout << "[DELETE] going on..." << std::endl;
	ti->__kv_store[tinfo->key][0]->__free = true;
        pthread_rwlock_unlock(&rwlock);
        pthread_barrier_wait(&barrier);
	return 0;
}

} // namespace ycsbc


