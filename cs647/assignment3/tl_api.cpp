#include "ti_api.hpp"
#include "tb_api.hpp"
#include "tl_api.hpp"

extern Tinylog *tl;
extern Tinyindex *ti;
extern uint32_t mode;
struct sigaction act;
clockid_t clock_id;
timer_t timer_id;
struct sigevent clock_sig_event;
struct itimerspec timer_value;
int ret;
sigset_t new_set, old_set;

void append(std::string key, std::string value){                                 
        std::string pairs_buf = "";

        pairs_buf += key + "," + value + "\n";

        char *tmp_data;
 
        if(posix_memalign((void**)&tmp_data, ALIGNMENT, 10 * 1024 * 1024)){
                std::cout << "[APPEND] *ERROR*: posix_memalign failed!" << std::endl;
                return;
        }                                                                   
                                                                                      
        strcpy(tmp_data, pairs_buf.c_str());
         
	int x = -1;
        if((x = pwrite(tl->fd, (const void *)(tmp_data), 10 * 1024 * 1024, tl->offset)) == -1){
                std::cout << "[APPEND] *ERROR* : Writing file unsuccessful!" << std::endl;
                return;                                                                    
        }    	
	tl->offset += ALIGNMENT;
}

void checkpoint_metadata(){
        std::string pairs_buf = "";

        for(auto x : ti->__kv_store)                                               
                for(auto y : x.second){
                        if(y)
                                pairs_buf += x.first + "," + std::to_string(y->index()) + "\n";
                }
        
        int fd;
	if(!mode)
		fd = open("device/blobs/pairs.txt", O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
	else
		fd = open("device/raw/pairs.txt", O_RDWR|O_CREAT|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR);
        int x;

        if(fd == -1){
                std::cout << "[PERSIST] *ERROR* : Opening file unsuccessful!" << std::endl;
                return;
        }

        if(!ti->allocated && (x = fallocate(fd, 0, 0, 10 * 1024 * 1024)) == -1){
                std::cout << "[PERSIST] *ERROR* : Allocating file unsuccessful!" << std::endl;
                return;
        }       
        else
                ti->allocated = true;
                       
        char *tmp_data;                         
                                                        
        if(posix_memalign((void**)&tmp_data, ALIGNMENT, 10 * 1024 * 1024)){
                std::cout << "[PERSIST] *ERROR*: posix_memalign failed!" << std::endl;
                return;
        }
        
        strcpy(tmp_data, pairs_buf.c_str());
        
	if((x = pwrite(fd, (const void *)(tmp_data), 10 * 1024 * 1024, 0)) == -1){
                std::cout << "[PERSIST] *ERROR* : Writing file unsuccessful!" << std::endl;
                return;
        }
}
