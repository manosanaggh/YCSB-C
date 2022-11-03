#include "ti_api.hpp"
#include "tb_api.hpp"
#include "tl_api.hpp"

extern Tinylog *tl;

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
