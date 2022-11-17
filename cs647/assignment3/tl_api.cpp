#include "ti_api.hpp"
#include "tb_api.hpp"
#include "tl_api.hpp"

extern Tinylog *tl;
extern Tinyindex *ti;
extern uint32_t mode, blob_size, global_dev_offset;
struct sigaction act;
clockid_t clock_id;
timer_t timer_id;
struct sigevent clock_sig_event;
struct itimerspec timer_value;
int ret;
sigset_t new_set, old_set;
extern std::vector<Tinyblob*> blobs;
extern long raw_size;

void append(std::string wal_buf){                                 
        char *tmp_data;
 
        if(posix_memalign((void**)&tmp_data, ALIGNMENT, 10 * 1024L * 1024L)){
                std::cout << "[APPEND] *ERROR*: posix_memalign failed!" << std::endl;
                return;
        }                                                                   
                                                                                      
        strcpy(tmp_data, wal_buf.c_str());
         
	int x = -1;
        if((x = pwrite(tl->fd, (const void *)(tmp_data), 10 * 1024L * 1024L, tl->offset)) == -1){
                std::cout << "[APPEND] *ERROR* : Writing file unsuccessful!" << std::endl;
                return;                                                                    
        }    	
	tl->offset += blob_size;
}

void handler(int sig, siginfo_t *info, void *ucontext){ 
        //checkpoint_metadata();
	std::cout << "handler" << std::endl;
	sigprocmask(SIG_UNBLOCK, &new_set, &old_set);
} 

void set_alarm(){
        memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO;
	ret = sigaction(SIGVTALRM, &act, NULL);
        assert(ret == 0);
	sigemptyset( &new_set );
        sigaddset( &new_set, SIGVTALRM );

        clock_id = CLOCK_MONOTONIC;
        memset(&clock_sig_event, 0, sizeof( struct sigevent));
        clock_sig_event.sigev_notify = SIGEV_SIGNAL;
        clock_sig_event.sigev_signo = SIGVTALRM;
        clock_sig_event.sigev_notify_attributes = NULL;

        ret = timer_create(clock_id, &clock_sig_event, &timer_id);
        assert(ret == 0);

        memset(&timer_value, 0, sizeof(struct itimerspec));
        timer_value.it_interval.tv_sec = 0;
        timer_value.it_interval.tv_nsec = INTERVAL * 1000000;

        timer_value.it_value.tv_sec = 0;
        timer_value.it_value.tv_nsec = INTERVAL * 1000000;

        ret = timer_settime(timer_id, 0, &timer_value, NULL);
        assert(ret == 0);
}

void checkpoint_metadata(){
        std::string pairs_buf = "";

        for(auto x : ti->__kv_store)                                               
                for(auto y : x.second){
                        if(y)
                                pairs_buf += x.first + "," + std::to_string(y->index()) + "\n";
                }
        
        //int fd;
	if(!mode){
		if(ti->fd == -1)
			ti->fd = open("/mnt/fmap/device/blobs/pairs.txt", O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
	}
	else{
		if(ti->fd == -1)
			ti->fd = open("/mnt/fmap/device/raw/pairs.txt", O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
	}
        int x;

        if(ti->fd == -1){
                std::cout << "[CHECKPOINT] *ERROR* : Opening file unsuccessful!" << std::endl;
                return;
        }

        if(!ti->allocated && (x = fallocate(ti->fd, 0, 0, raw_size)) == -1){
                std::cout << "[CHECKPOINT] *ERROR* : Allocating file unsuccessful!" << std::endl;
                return;
        }       
        else
                ti->allocated = true;
                       
        char *tmp_data;                         
                                                        
        if(posix_memalign((void**)&tmp_data, ALIGNMENT, 10 * 1024L * 1024L)){
                std::cout << "[CHECKPOINT] *ERROR*: posix_memalign failed!" << std::endl;
                return;
        }
        
        strcpy(tmp_data, pairs_buf.c_str());
        
	if((x = pwrite(ti->fd, (const void *)(tmp_data), 10 * 1024L * 1024L, 0)) == -1){
                std::cout << "[CHECKPOINT] *ERROR* : Writing file unsuccessful!" << std::endl;
                return;
        }
}

int truncate(){
        if(mode)
		remove("/mnt/fmap/device/raw/wal.log");
        else
		remove("/mnt/fmap/device/blobs/wal.log");
	return 0;
}

Tinyindex *replay(){
	Tinyindex *ti = NULL;
	if(mode && access("/mnt/fmap/device/raw/wal.log", F_OK) != 0)
		return ti;
	else if(!mode && access("/mnt/fmap/device/blobs/wal.log", F_OK) != 0)
		return ti;
	ti = new Tinyindex();
        char *tmp_data;
	uint32_t wal_offset = 0;
	int x;
        //int fd;
	if(!mode){
                if(tl->fd == -1)
			if((tl->fd = open("/mnt/fmap/device/blobs/wal.log", O_RDWR|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR)) == -1)
                        	std::cout << "[REPLAY] Error with open" << std::endl;
	}
	else{
		if(tl->fd == -1)
                	if((tl->fd = open("/mnt/fmap/device/raw/wal.log", O_RDWR|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR)) == -1)
                        	std::cout << "[REPLAY] Error with open" << std::endl;
	}

        if(posix_memalign((void**)&tmp_data, ALIGNMENT, 10 * 1024L * 1024L)){   
                std::cout << "[REPLAY] *ERROR*: posix_memalign failed!" << std::endl;                 
                return NULL;                                                                   
        }
        if((x = pread(tl->fd, (void *)(tmp_data), blob_size, wal_offset)) == -1){                              
                std::cout << "[REPLAY] *ERROR* : Reading file unsuccessful!" << std::endl;
                return NULL;                                              
        }

	while(strcmp(tmp_data,"") != 0){
		std::string input(tmp_data);
    		std::vector<std::string> result;
   		boost::split(result, input, boost::is_any_of("\n"));
 
    		for (int i = 0; i < (int)(result.size()-1); i++){
                	Tinyblob *tb = new Tinyblob();
                	tb->__free = false;
                	blobs.push_back(tb);
                	if(!mode){
                        	tb->__open();
                        	tb->setOffset(0);
                	}
                	else{
                        	tb->__open((char*)"/mnt/fmap/device/raw/file.txt");
                        	tb->setOffset(global_dev_offset);
                	}
                	tb->__persisted = false;
                	global_dev_offset += blob_size;
        		std::string input2(result[i]);                                                                                        				std::vector<std::string> result2;         
        		boost::split(result2, input2, boost::is_any_of(","));
        		std::string key;
        		key = result2[0];
			strcpy((char*)tb->__io_buffer, result2[1].c_str());
			//std::cout << result2[0] << "," << result2[1] << std::endl;
        		ti->__kv_store[key].push_back(tb);        

    		}
		wal_offset += blob_size;
        	if((x = pread(tl->fd, (void *)(tmp_data), blob_size, wal_offset)) == -1){                            
                	std::cout << "[REPLAY] *ERROR* : Reading file unsuccessful!" << std::endl;
                	return NULL;                                              
        	}
	}
	return ti;
}
