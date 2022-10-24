#include "ti_api.hpp"
#include "tb_api.hpp"
#include "db/cs647_db.h"

extern Tinyindex *ti;
extern pthread_rwlock_t rwlock;
extern pthread_barrier_t barrier;
extern uint32_t mode, nprocs, blob_size, raw_size, g_primary_offset, g_index, g_secondary_offset_b4, g_secondary_offset_after;
extern std::string g_sec_key;
std::vector<std::string> keys;
std::vector<std::string> values;

int main(int argc, char **argv){
	pthread_t threads[1024];

	mode = atoi(argv[1]);
        nprocs = atoi(argv[2]); 
	blob_size = atoi(argv[3]);
	raw_size = atoi(argv[4]);

        if (pthread_rwlock_init(&rwlock, NULL) != 0) {                             
                printf("\n rwlock init has failed\n");                                                        
                return 1;                                                                                 
        }                                                                                    
        pthread_barrier_init(&barrier, NULL, nprocs+1);    

	ti = new Tinyindex();

	uint32_t offset = 0, c = 0;
	std::vector<Tinyblob*> *pv = NULL;
	std::vector<ycsbc::DB::KVPair> input, result;
	ycsbc::CS647DB db;
	db.Init();
	recover("device/raw/pairs.txt"); //comment out to run persist

	/* PERSIST START */
/*
        for(int i = 0; i < nprocs; i++) //uncomment to run with allocate      
                pthread_create(&threads[i], NULL, (void* (*)(void*))&tb_allocate_blob, NULL); //uncomment to run with allocate
	pthread_barrier_wait(&barrier);

	for(int i = 0; i < 15; i++){
		keys.push_back("g" + std::to_string(i));
		values.push_back("Value" + std::to_string(i));
	}

        pthread_barrier_destroy(&barrier);
        pthread_barrier_init(&barrier, NULL, 2);

	for(int i = 0; i < 10; i++){
		input.push_back(std::make_pair(keys[i], values[i]));
		if(db.Insert("", "g"+std::to_string(i), input) == -1)
			std::cout << "[INSERT] Key already exists!" << std::endl;
		input.clear();
	}

        if(db.Insert("", "g0", input) == -1)
                std::cout << "[INSERT] Key already exists!" << std::endl;

	input.clear();

        for(int i = 0; i < 10; i++)
                input.push_back(std::make_pair(keys[i], values[i]));

        if(db.Update("", "g0", input) == -1)
                std::cout << "[UPDATE] Key does not exist!" << std::endl;

        if(db.Update("", "g80", input) == -1)
                std::cout << "[UPDATE] Key does not exist!" << std::endl;

	if(db.Read("", "g0", NULL, result) == -1)
		std::cout << "[READ] Key does not exist!" << std::endl;

        if(db.Read("", "g80", NULL, result) == -1) 
                std::cout << "[READ] Key does not exist!" << std::endl;

	for(auto x : result)
		std::cout << "[READ] (" << x.first << "," << x.second << ")" << std::endl;

	for(int i = 0; i < 3; i++){
		Thread_info *tinfo = new Thread_info();
		if(i == 2)
			tinfo->key = "data10";
		else
			tinfo->key = keys[i];
		if(db.Delete("", tinfo->key) == 0)
			std::cout << "[DELETE] Successful on key " << tinfo->key << std::endl;
		else
			std::cout << "[DELETE] Key " << tinfo->key << " does not exist!" << std::endl;
        }

	persist("device/raw/pairs.txt");
*/

	/* PERSIST END */
	
	std::vector<std::vector<ycsbc::DB::KVPair>> result2;

	if(db.Scan("", "g9", 5, NULL, result2) == -1)
		std::cout << "[SCAN] Error with scan - check further!" << std::endl;

	for(auto x : result2){
		for(auto y : x){
			std::cout << "[SCAN] (" << y.first << "," << y.second << ")" << std::endl; 
		}
	}
	
	for(auto x : ti->__kv_store)
		for(auto y : x.second)
			std::cout << "Key: " << x.first << " | Value of blob " << y->index() << ": " << (char*)y->__io_buffer << std::endl;

        pthread_barrier_destroy(&barrier);
        pthread_rwlock_destroy(&rwlock);
	return 0;
}
