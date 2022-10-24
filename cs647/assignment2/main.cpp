#include "ti_api.hpp"
#include "tb_api.hpp"

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

	std::unordered_map<std::string, std::string> value;
	uint32_t offset = 0, c = 0;
	std::vector<Tinyblob*> *pv = NULL;

//	recover("device/raw/pairs.txt");

        for(int i = 0; i < nprocs; i++) //uncomment to run with allocate
                pthread_create(&threads[i], NULL, (void* (*)(void*))&tb_allocate_blob, NULL); //uncomment to run with allocate
	pthread_barrier_wait(&barrier);
	
	for(int i = 0; i < 15; i++){
		keys.push_back("g" + std::to_string(i));
		values.push_back("Value" + std::to_string(i));
	}

        pthread_barrier_destroy(&barrier);
        pthread_barrier_init(&barrier, NULL, 2);

	for(int i = 0; i < keys.size(); i++){
		Thread_info *tinfo = new Thread_info();
		tinfo->key = keys[i];
		tinfo->value = values[i];
		pthread_create(&threads[i], NULL, (void* (*)(void*))&put, tinfo);
		pthread_barrier_wait(&barrier);
		if(tinfo->result == -1)
			std::cout << "[PUT] No free blobs!" << std::endl;
	}

		if((pv = get("g1")) != NULL){
			for(auto x : *pv){
				if(!strcmp((const char*)x->__io_buffer, "Value0")){
					c = 1;
					std::cout << "[GET] Found value: " << (char*)x->__io_buffer << " in key g1" << std::endl;
				}
			}
			if(!c)
				std::cout << "[GET] Value not found in key g1!" << std::endl;
		}
		else
			std::cout << "[GET] Key does not exist!" << std::endl;

                if((pv = get("g80")) != NULL){
                        for(auto x : *pv){
                                if(!strcmp((const char*)x->__io_buffer, "Value0")){
                                        c = 1;
                                        std::cout << "[GET] Found value: " << (char*)x->__io_buffer << " in key g0" << std::endl;
                                }
                        }
                        if(!c)
                                std::cout << "[GET] Value not found in key g0!" << std::endl;
                }
                else
                        std::cout << "[GET] Key does not exist!" << std::endl;
	
	/*pthread_barrier_destroy(&barrier);
	pthread_barrier_init(&barrier, NULL, 2);

	for(int i = 0; i < 3; i++){
		Thread_info *tinfo = new Thread_info();
		if(i < 2)
			tinfo->key = "g" + std::to_string(i);
		else
			tinfo->key = "g80";
		pthread_create(&threads[i], NULL, (void* (*)(void*))&get, tinfo);
		pthread_barrier_wait(&barrier);
		if(!tinfo->pv.empty()){
                        for(auto x : tinfo->pv){
                                if(!strcmp((const char*)x->__io_buffer, "Value0")){
                                        c = 1;
                                        std::cout << "[GET] Found value: " << (char*)x->__io_buffer << " in key " << tinfo->key << std::endl;
                                }
                        }
                        if(!c)
                                std::cout << "[GET] Value not found in key " << tinfo->key << std::endl;
                }
                else
                        std::cout << "[GET] Key does not exist!" << std::endl;
	}*/

	pthread_barrier_destroy(&barrier);
	pthread_barrier_init(&barrier, NULL, 2);

	for(int i = 0; i < 3; i++){
		Thread_info *tinfo = new Thread_info();
		if(i == 2)
			tinfo->key = "data10";
		else
			tinfo->key = keys[i];
                pthread_create(&threads[i], NULL, (void* (*)(void*))&erase, tinfo);
		pthread_barrier_wait(&barrier);
		if(tinfo->result == 0)
			std::cout << "[ERASE] Successful on key " << tinfo->key << std::endl;
		else
			std::cout << "[ERASE] Key " << tinfo->key << " does not exist!" << std::endl;
        }

	persist("device/raw/pairs.txt");

        ti->scanner = scan_init();
        if(!ti->scanner.is_open()) {                                                             
                perror("Error open");                                                    
        }  
	std::cout << "[SCAN_INIT] First pair: " << ti->next_pair << std::endl;

	pthread_barrier_destroy(&barrier);	
	pthread_barrier_init(&barrier, NULL, 2);

	for(int i = 0; i < 3; i++){
		pthread_create(&threads[i], NULL, (void* (*)(void*))&get_next, (void*)&ti->scanner);
		pthread_barrier_wait(&barrier);
		std::cout << "[GET_NEXT] Next pair: " << ti->next_pair << std::endl;
	}

	std::cout << "[GET_SCAN_KEY] " << get_scan_key(ti->next_pair) << std::endl;

	c = 0;

                if((pv = get_scan_value(ti->next_pair)) != NULL){
                        for(auto x : *pv){
                                if(!strcmp((const char*)x->__io_buffer, "Value7")){
                                        c = 1;
                                        std::cout << "[GET_SCAN_VALUE] Found value: " << (char*)x->__io_buffer 
						<< " in pair " << ti->next_pair 
						<< std::endl;
                                }
                        }                                               
                        if(!c)                                                      
                                std::cout << "[GET_SCAN_VALUE] Value not found in pair " << ti->next_pair << std::endl;
                }                                                              
                else                                                               
                        std::cout << "[GET_SCAN_VALUE] Key does not exist!" << std::endl;  
	
	int r;
	if((r = close_scanner(&ti->scanner)) == -1)
			std::cout << "[CLOSE_SCANNER] *Error* Closing the scanner failed!";	

	for(auto x : ti->__kv_store)
		for(auto y : x.second)
			std::cout << "Key: " << x.first << " | Value of blob " << y->index() << ": " << (char*)y->__io_buffer << std::endl;

        pthread_barrier_destroy(&barrier);
        pthread_rwlock_destroy(&rwlock);

	return 0;
}
