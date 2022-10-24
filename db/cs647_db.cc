//
//  cs647_db.cc
//  YCSB-C        
//                                                       
//  Created by Manos Anagnostakis on 10/23/22.           
//  Copyright (c) 2022 Manos Anagnostakis <manosanag@csd.uoc.gr>.
//  

#include "cs647_db.h"
#include "cs647/assignment2/ti_api.hpp"

extern pthread_barrier_t barrier;
extern Tinyindex *ti;

namespace ycsbc {

int CS647DB::Read(const std::string &table, const std::string &key,
        const std::vector<std::string> *fields,
        std::vector<KVPair> &result) {

	std::vector<Tinyblob*> *pv = NULL;

        if((pv = get(key)) != NULL){
                for(auto x : *pv)
			result.push_back(std::make_pair(key, std::string((char*)x->__io_buffer)));
		return 0;
	}
	return -1;
}

int CS647DB::Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result){
	if(ti->__kv_store[key].empty())
		return -1;

	std::vector<KVPair> result2;
        ti->scanner = scan_init();
        if(!ti->scanner.is_open()) {                                                             
                perror("Error open");      
		return -1;
        }  
	for(int i = 0; i < len; i++){
		for(auto x : *(get_scan_value(ti->next_pair)))
			result2.push_back(std::make_pair(get_scan_key(ti->next_pair), std::string((char*)x->__io_buffer)));
		result.push_back(result2);
		if(get_next(&ti->scanner) == -1)
			return -1;
	}
	return close_scanner(&ti->scanner);
}

int CS647DB::Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){
        if(ti->__kv_store[key].empty())
                return -1;
        pthread_t threads[1024];

        pthread_barrier_destroy(&barrier);
        pthread_barrier_init(&barrier, NULL, 2);

        for(int i = 0; i < values.size(); i++){
                Thread_info *tinfo = new Thread_info();                 
                tinfo->key = key;                              
                tinfo->value = values[i].second;                  
                pthread_create(&threads[i], NULL, (void* (*)(void*))&put, tinfo);
                pthread_barrier_wait(&barrier);
                if(tinfo->result == -1)
                        std::cout << "[UPDATE] No free blobs!" << std::endl;
        }
        return 0;
}

int CS647DB::Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){
	if(!ti->__kv_store[key].empty())
		return -1;
	pthread_t threads[1024];

        pthread_barrier_destroy(&barrier);
        pthread_barrier_init(&barrier, NULL, 2);

	for(int i = 0; i < values.size(); i++){
                Thread_info *tinfo = new Thread_info();                 
                tinfo->key = key;                              
                tinfo->value = values[i].second;                  
                pthread_create(&threads[i], NULL, (void* (*)(void*))&put, tinfo);
                pthread_barrier_wait(&barrier);
                if(tinfo->result == -1)
                        std::cout << "[INSERT] No free blobs!" << std::endl;
	}
	return 0;
}       

int CS647DB::Delete(const std::string &table, const std::string &key){
        Thread_info *tinfo = new Thread_info();                                               
        tinfo->key = key;

	if(erase(tinfo) == -1){
		return -1;
	}
	return 0;
}

} // namespace ycsbc


