#include "tb_api.hpp"

uint32_t mode, nprocs, blob_size, raw_size, global_dev_offset = 0;

std::vector<Tinyblob*> blobs;
pthread_mutex_t lock;
pthread_barrier_t barrier;

int tb_allocate_blob(){
        Tinyblob *tb = new Tinyblob();
        //pthread_mutex_lock(&lock); 
        blobs.push_back(tb);                  
        //pthread_mutex_unlock(&lock); 
	if(!mode){
		tb->__open();
		if(tb->fd() < 0){
			blobs[tb->index()]->ti->result = -1;
			return -1;
		}
		int x = fallocate(tb->fd(), 0, 0, blob_size);
		if(x < 0){
                        blobs[tb->index()]->ti->result = -1;         
                        return -1;
		}
	//	pthread_barrier_wait(&barrier);
		blobs[tb->index()]->ti->result = tb->index();
		return tb->index();
	}
	//pthread_mutex_lock(&lock);
	tb->__open((char*)"device/raw/file.txt");
	int x;
        if (access("device/raw/file.txt", F_OK) == 0 && tb->created){      
        	if((x = fallocate(tb->fd(), 0, 0, raw_size * 1024 * 1024)) == -1){
			std::cout << "[TB_ALLOCATE_BLOB] Problem with falllocate!" << std::endl;
			return -1;
		}
	}
	if(tb->fd() < 0){
		blobs[tb->index()]->ti->result = -1;
		return -1;
	}
        tb->setOffset(global_dev_offset);
        global_dev_offset += blob_size;
	//pthread_mutex_unlock(&lock);
	//pthread_barrier_wait(&barrier);
	blobs[tb->index()]->ti->result = tb->index();
	return tb->index();
}

void tb_free_blob(int index){
	char *tmp_data;
	int x;
        if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))
                std::cout << "posix_memalign failed!" << std::endl;
	if(!mode){
		if(blobs[index]->fd() >= 0)
			if(index >= 0 && index <= (int)Tinyblob::cnt_blob()){ 
				pthread_mutex_lock(&lock);
                        	memcpy(tmp_data, "0", 2);
				pthread_mutex_unlock(&lock);
                        	if((x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, 0)) == 0)
                                	std::cout << "[TB_WRITE_BLOB] Writen 0 bytes" << std::endl;
				fallocate(blobs[index]->fd(), FALLOC_FL_PUNCH_HOLE, 0, blob_size);
				blobs[index]->__free = true;
				blobs[index]->__close();
			}
	}
        if(blobs[index]->fd() >= 0)                                                       
                if(index >= 0 && index <= (int)Tinyblob::cnt_blob())                      
                        blobs[index]->__close();
}

int tb_write_blob(int index, void *data){  
	if(blobs[index]->__persisted)
		return 0;
	char *tmp_data;
	int x;
	if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))
		std::cout << "posix_memalign failed!" << std::endl;
        if(!mode){
        	if(pread(blobs[index]->fd(), tmp_data, blob_size, 0) == -1)
                	std::cout << "read failed!" << std::endl;
		if(tmp_data[0] != '1')
			blobs[index]->__free = true;
                if(blobs[index]->fd() >= 0 && blobs[index]->is_free()){       
			memcpy(tmp_data, "1", 2);
			if((x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, 0)) == 0)
				std::cout << "[TB_WRITE_BLOB] Writen 0 bytes" << std::endl;
			memcpy(tmp_data, data, blob_size+1);
                        if((x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, ALIGNMENT)) == 0) 
                                std::cout << "[TB_WRITE_BLOB] Writen 0 bytes" << std::endl;  
                        free(blobs[index]->__io_buffer);                              
                        if(posix_memalign(&(blobs[index]->__io_buffer), ALIGNMENT, blob_size))
				std::cout << "posix_memalign failed!" << std::endl;
                        tb_flush();       
			blobs[index]->__free = false;
                        return x;
                }
		else{
			Tinyblob *tb = new Tinyblob();
			tb->__open();
			blobs.push_back(tb);
                	if((x = fallocate(tb->fd(), 0, 0, blob_size)) < 0)                                  
                        	return -1;   
			if((x = tb_write_blob(tb->index(), data)) == 0)
				std::cout << "[TB_WRITE_BLOB] Writen 0 bytes" << std::endl;	
			return x;
		}
        }

        if(pread(blobs[index]->fd(), tmp_data, blob_size, blobs[index]->offset()+0) == -1)
		std::cout << "read failed!" << std::endl;
	
	if(tmp_data[0] != '1')
                blobs[index]->__free = true;
	
        if(blobs[index]->fd() >= 0 && blobs[index]->is_free()){
                memcpy(tmp_data, "1", 2);
                if((x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, blobs[index]->offset()+0)) == 0)     
                        std::cout << "[TB_WRITE_BLOB] Writen 0 bytes" << std::endl;
                memcpy(tmp_data, data, blob_size+1);     
                if((x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, blobs[index]->offset()+ALIGNMENT)) == 0)
                        std::cout << "[TB_WRITE_BLOB] Writen 0 bytes" << std::endl; 
                free(blobs[index]->__io_buffer);
                if(posix_memalign(&(blobs[index]->__io_buffer), ALIGNMENT, blob_size))
                	std::cout << "posix_memalign failed!" << std::endl;
		tb_flush();
		blobs[index]->__free = false;
                return x;                    
        } 
        else{    
        	Tinyblob *tb = new Tinyblob();
                tb->__open((char*)"device/raw/file.txt");      
                tb->setOffset((blobs.size() * blob_size) + blob_size);
                blobs.push_back(tb);      
                if((x = tb_write_blob(tb->index(), data)) == 0)                         
                        std::cout << "[TB_WRITE_BLOB] Writen 0 bytes" << std::endl;
                return x;
        }
        return -1;                        
} 

int tb_read_blob(void *args){
	struct thread_info *ti = (struct thread_info*)args;
        free(blobs[ti->i]->__io_buffer);
        if(posix_memalign(&(blobs[ti->i]->__io_buffer), ALIGNMENT, blob_size))
		std::cout << "posix_memalign failed!" << std::endl;
        char *tmp_data;                                            
        if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))                       
                std::cout << "posix_memalign failed!" << std::endl;  
	if(!mode)
		if(blobs[ti->i]->fd() >= 0){
			int x = pread(blobs[ti->i]->fd(), tmp_data, blob_size, ALIGNMENT);
			pthread_mutex_lock(&lock);
			memcpy(*((char**)ti->buffer), tmp_data, blob_size+1);
			pthread_mutex_unlock(&lock);
        		if(ti->parallel)                       
                		pthread_barrier_wait(&barrier); 
			ti->result = x;
			return x;
		}
        if(blobs[ti->i]->fd() >= 0){
                int x = pread(blobs[ti->i]->fd(), tmp_data, blob_size, blobs[ti->i]->offset()+ALIGNMENT);
                pthread_mutex_lock(&lock);
                memcpy(*((char**)ti->buffer), tmp_data, blob_size+1);
                pthread_mutex_unlock(&lock);
        	if(ti->parallel)                       
                	pthread_barrier_wait(&barrier); 
                ti->result = x;
        	return x;
        }
	if(ti->parallel)
		pthread_barrier_wait(&barrier);
	ti->result = -1;
	return -1;
}

void tb_flush(){
	uint32_t i;
	for(i = 0; i < Tinyblob::cnt_blob(); i++){
		int fd = blobs[i]->fd();
		if(fd >= 0){
			if(fsync(fd) == 0) 
				return;
			else 
				exit(EXIT_FAILURE);
		}
	}
}

void tb_shutdown(){
        for(auto tb : blobs){
                int x;              
                if((x = tb_write_blob(tb->index(), tb->__io_buffer)) == -1)
                        std::cout << "[SHUTDOWN] *ERROR* : tb_write_blob failed!" << std::endl;
        }
	tb_flush();
}

void tb_init(char *location){
DIR *dir;
struct dirent *ent;
if ((dir = opendir (location)) != NULL) {
  while ((ent = readdir (dir)) != NULL) {
	if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, "pairs.txt") == 0){
		continue;
	}
	if(!mode){
                char *tmp_data;
                if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))                       
                        std::cout << "posix_memalign failed!" << std::endl; 
                char final_location[30] = "";
                strcat(final_location, location);
                strcat(final_location, ent->d_name);
		std::cout << final_location << std::endl;
                int fd;
                if((fd = open(final_location, O_RDWR|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR)) == -1)
                        std::cout << "[TB_INIT] Error with open" << std::endl;
                if(pread(fd, tmp_data, blob_size, 0) == -1)
                        std::cout << "[TB_INIT] read failed!" << std::endl;
                Tinyblob *tb = new Tinyblob();
                if(tmp_data[0] == '0')
                        tb->__free = true;
                else
                        tb->__free = false;
                blobs.push_back(tb);
                tb->__open();
		tb->__persisted = true;
                if(pread(tb->fd(), tb->__io_buffer, blob_size, tb->offset()+ALIGNMENT) == -1)
                        std::cout << "[TB_INIT] read failed!" << std::endl;
	}
	else{
		char *tmp_data;
                if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))                       
                        std::cout << "posix_memalign failed!" << std::endl; 
                char final_location[30] = "";
                strcat(final_location, location);
                strcat(final_location, ent->d_name);
		int fd;
		if((fd = open(final_location, O_RDWR|O_DIRECT|O_DSYNC, S_IRUSR|S_IWUSR)) == -1)
			std::cout << "[TB_INIT] Error with open" << std::endl;
                if(pread(fd, tmp_data, blob_size, global_dev_offset+0) == -1)
                        std::cout << "[TB_INIT] read failed!" << std::endl;
		while(tmp_data[0] == '0' || tmp_data[0] == '1'){
			Tinyblob *tb = new Tinyblob();
			if(tmp_data[0] == '0')
				tb->__free = true;
			else
				tb->__free = false;
			blobs.push_back(tb);
			tb->__open(final_location);
			tb->setOffset(global_dev_offset);
			tb->__persisted = true;
                        if(pread(tb->fd(), tb->__io_buffer, blob_size, tb->offset()+ALIGNMENT) == -1)
                                std::cout << "[TB_INIT] read failed!" << std::endl;
			global_dev_offset += blob_size;
                        if(pread(tb->fd(), tmp_data, blob_size, global_dev_offset+0) == -1)
                                std::cout << "read failed!" << std::endl;
		}
	}
  }
  closedir (dir);
} else {
  perror ("");
  return;
}
}
