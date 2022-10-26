#include "ti_api.hpp"
#include "tb_api.hpp"

Tinyindex *ti = NULL;
pthread_rwlock_t rwlock;
extern pthread_barrier_t barrier;
extern uint32_t nprocs, blob_size;
uint32_t g_primary_offset = 0, g_index = 0, g_secondary_offset_b4 = 0, g_secondary_offset_after = 0;
std::string g_sec_key = "";
extern std::vector<Tinyblob*> blobs;

//Multithreaded
int put(void *args){
	Thread_info *tinfo = (Thread_info*)args;
	//pthread_rwlock_wrlock(&rwlock);
	int i;
	for(i = 0; i < ti->__kv_store[tinfo->key].size(); i++){
		Tinyblob *tb = ti->__kv_store[tinfo->key][i];
		if(tb->is_free()){
			strcpy((char*)tb->__io_buffer, tinfo->value.c_str());
			tb->__free = false;
			break;
		}
	}

	if(ti->__kv_store[tinfo->key].size() == 0 || i == ti->__kv_store[tinfo->key].size()){
		for(i = 0; i < blobs.size(); i++){
			if(blobs[i]->is_free()){
				ti->__kv_store[tinfo->key].push_back(blobs[i]);
				strcpy((char*)blobs[i]->__io_buffer, tinfo->value.c_str());
				std::cout << "Buffer: " << (char*)blobs[i]->__io_buffer << std::endl;
				blobs[i]->__free = false;
				break;
			}
		}
	}

	if(i == blobs.size()){
            //    pthread_rwlock_unlock(&rwlock);
              //  pthread_barrier_wait(&barrier);
		tinfo->result = -1;
		return -1;
	}
	//pthread_rwlock_unlock(&rwlock);
       // pthread_barrier_wait(&barrier);
	tinfo->result = 0;
	return 0;
}

std::vector<Tinyblob*> *get(std::string key){
	if(ti->__kv_store[key].empty())
		return NULL;
	return &(ti->__kv_store[key]);
}

//Multithreaded
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
        scanner.open("device/raw/pairs.txt");
        if(!scanner.is_open()) { 
                perror("Error open");                                                        
                exit(EXIT_FAILURE);
        }	
	std::getline(scanner, ti->next_pair);
	return scanner;
}

//Multithreaded
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
	std::string pairs_buf = "";

        for(auto x : ti->__kv_store)                                               
                for(auto y : x.second)        
			pairs_buf += x.first + "," + std::to_string(y->index()) + "\n";

	bool created = true;

	if (access(location, F_OK) != 0)
		created = false;

	int fd = open(location, O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
	int x;

	if(fd == -1){
		std::cout << "[PERSIST] *ERROR* : Opening file unsuccessful!" << std::endl;
		return;
	}

	if(!created)
		x = fallocate(fd, 0, 0, 1 * 1024 * 1024);

        if(x == -1){
                std::cout << "[PERSIST] *ERROR* : Allocating file unsuccessful!" << std::endl;
                return;
        }
    	
	char *tmp_data;

        if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size)){ 
                std::cout << "[PERSIST] *ERROR*: posix_memalign failed!" << std::endl;
		return;
	}
	
	strcpy(tmp_data, pairs_buf.c_str());

	if((x = pwrite(fd, (const void *)(tmp_data), blob_size, 0)) == -1){
		std::cout << "[PERSIST] *ERROR* : Writing file unsuccessful!" << std::endl;
		return;
	}

	// Debug
        for(auto x : ti->__kv_store)
                for(auto y : x.second)                                                            
                        std::cout << "Key: " << x.first << " | Value of blob " << y->index() << ": " << (char*)y->__io_buffer << std::endl;

	tb_shutdown();
}

void recover(char *location){
	tb_init("device/raw/");

        int fd = open(location, O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);

        if(fd == -1){
                std::cout << "[RECOVER] *ERROR* : Opening file unsuccessful!" << std::endl;
                return;
        }

        char *tmp_data;
	int x;

        if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size)){ 
                std::cout << "[RECOVER] *ERROR*: posix_memalign failed!" << std::endl;
                return;
        }
        
        if((x = pread(fd, (void *)(tmp_data), blob_size, 0)) == -1){
                std::cout << "[RECOVER] *ERROR* : Reading file unsuccessful!" << std::endl;
                return;
        }

	//Debug
	std::cout << tmp_data << std::endl;

	std::string input(tmp_data);
    std::vector<std::string> result;
    boost::split(result, input, boost::is_any_of("\n"));
 
    for (int i = 0; i < result.size()-1; i++){
        std::string input2(result[i]);                                                                                          
    	std::vector<std::string> result2;         
    	boost::split(result2, input2, boost::is_any_of(","));
	std::string key;
	key = result2[0];
	ti->__kv_store[key].push_back(blobs[std::stoi(result2[1])]);			
    }

    	//Debug
        for(Tinyblob *tb : blobs){                       
                tb->printTb();
        }
}
