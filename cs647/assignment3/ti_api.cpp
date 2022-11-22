#include "ti_api.hpp"
#include "tb_api.hpp"
#include "tl_api.hpp"

Tinyindex *ti = NULL;
Tinylog *tl = NULL;
pthread_rwlock_t rwlock;
extern pthread_barrier_t barrier;
extern uint32_t blob_size, mode;
extern std::vector<Tinyblob*> blobs;
extern long raw_size;

int put(void *args){
	Thread_info *tinfo = (Thread_info*)args;
	//pthread_rwlock_wrlock(&rwlock);
	int i;
	tb_allocate_blob();
	for(i = 0; i < (int)ti->__kv_store[tinfo->key].size(); i++){
		Tinyblob *tb = ti->__kv_store[tinfo->key][i];
		if(tb->is_free()){
			strcpy((char*)tb->__io_buffer, tinfo->value.c_str());
			tb->__free = false;
			break;
		}
	}

	if(ti->__kv_store[tinfo->key].size() == 0 || i == (int)ti->__kv_store[tinfo->key].size()){
		for(i = 0; i < (int)blobs.size(); i++){
			if(blobs[i]->is_free()){
				ti->__kv_store[tinfo->key].push_back(blobs[i]);
				strcpy((char*)blobs[i]->__io_buffer, tinfo->value.c_str());
				blobs[i]->__free = false;
				break;
			}
		}
	}

	if(i == (int)blobs.size()){
            //    pthread_rwlock_unlock(&rwlock);
              //  pthread_barrier_wait(&barrier);
		tinfo->result = -1;
		return -1;
	}
	//pthread_rwlock_unlock(&rwlock);
        // pthread_barrier_wait(&barrier);
	if(tl->buf_sz < 4){
		tl->wal_buf += tinfo->key + "\n" + tinfo->value;
		tl->buf_sz++;
	}
	else{
		tl->wal_buf += tinfo->key + "\n" + tinfo->value;
		append(tl->wal_buf);
		tl->wal_buf = "";
		tl->buf_sz = 0;
		checkpoint_metadata();
	}

	tinfo->result = 0;
	return 0;
}

std::vector<Tinyblob*> *get(std::string key){
	if(ti->__kv_store[key].empty())
		return NULL;
	return &(ti->__kv_store[key]);
}

int erase(void *args){
	Thread_info *tinfo = (Thread_info*)args;
        uint32_t g = 0;
	//pthread_rwlock_wrlock(&rwlock);
        for(auto it = ti->__kv_store.begin(); it != ti->__kv_store.end(); it++){
                if(it->first == tinfo->key){
                        ti->__kv_store.erase(it);
                        g = 1;
                        break;
                }
        }
        if(g){
		tinfo->result = 0;
	//	pthread_rwlock_unlock(&rwlock);
	//	pthread_barrier_wait(&barrier);
                return 0;
	}
        else{
		tinfo->result = -1;
	//	pthread_rwlock_unlock(&rwlock);
	//	pthread_barrier_wait(&barrier);
                return -1;
	}
}

std::ifstream scan_init(void){
	std::ifstream scanner;
        scanner.open("/mnt/fmap/device/raw/pairs.txt");
        if(!scanner.is_open()) { 
		std::cout << "[SCAN_INIT] Error scanner not open!" << std::endl;                                                        
                exit(EXIT_FAILURE);
        }	
	std::getline(scanner, ti->next_pair);
	return scanner;
}

int get_next(std::ifstream *scanner){
	//pthread_rwlock_wrlock(&rwlock);
	if(std::getline(*scanner, ti->next_pair)){
	//	pthread_rwlock_unlock(&rwlock);
	//	pthread_barrier_wait(&barrier);
		return 0;
	}
	//pthread_rwlock_unlock(&rwlock);
	//pthread_barrier_wait(&barrier);
	return -1;
}

std::string get_scan_key(std::string current_pair){
	std::vector<std::string> result2;
        boost::split(result2, current_pair, boost::is_any_of(","));	
	return result2[0];
}

std::vector<Tinyblob*> *get_scan_value(std::string current_pair){
        std::vector<std::string> result2;                                                                
        boost::split(result2, current_pair, boost::is_any_of(",")); 
	return &(ti->__kv_store[result2[0]]);
}

int close_scanner(std::ifstream *scanner){
	if(scanner){
		scanner->close();
		return 0;
	}
	return -1;
}

void persist(char *location){
	checkpoint_metadata();

	for(Tinyblob *tb : blobs)                       
                tb->printTb();

	std::cout << blobs.size() << std::endl;

	tb_shutdown();
}

void recover(char *location){
	if(mode) tb_init((char*)"/mnt/fmap/device/raw/");
	else tb_init((char*)"/mnt/fmap/device/blobs/");

        ti->fd = open(location, O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);

        if(ti->fd == -1){
                std::cout << "[RECOVER] *ERROR* : Opening file unsuccessful!" << std::endl;
                return;
        }

        char *tmp_data;
	int x;

        if(posix_memalign((void**)&tmp_data, ALIGNMENT, 10 * 1024L * 1024L)){ 
                std::cout << "[RECOVER] *ERROR*: posix_memalign failed!" << std::endl;
                return;
        }
        
        if((x = pread(ti->fd, (void *)(tmp_data), 10 * 1024L * 1024L, 0)) == -1){
                std::cout << "[RECOVER] *ERROR* : Reading file unsuccessful!" << std::endl;
                return;
        }

	std::string input(tmp_data);
    std::vector<std::string> result;
    boost::split(result, input, boost::is_any_of("\n"));
 
    for (int i = 0; i < (int)(result.size()-1); i++){
        std::string input2(result[i]);                                                                                          
    	std::vector<std::string> result2;         
    	boost::split(result2, input2, boost::is_any_of(","));
	std::string key;
	key = result2[0];
	ti->__kv_store[key].push_back(blobs[std::stoi(result2[1])]);			
    }

    	//Debug
    	std::cout << "[RECOVER]" << std::endl;
        for(Tinyblob *tb : blobs)                       
                tb->printTb();

	std::cout << blobs.size() << std::endl;
	free(tmp_data);
}
