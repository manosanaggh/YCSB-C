#include "tb_api.hpp"

uint32_t mode, nprocs, blob_size, io_size, raw_size, global_dev_offset = 0;

std::vector<Tinyblob*> blobs;
pthread_mutex_t lock;
pthread_barrier_t barrier;

int tb_allocate_blob(){
        Tinyblob *tb = new Tinyblob();
        pthread_mutex_lock(&lock); 
        blobs.push_back(tb);                  
        pthread_mutex_unlock(&lock); 
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
		pthread_barrier_wait(&barrier);
		blobs[tb->index()]->ti->result = tb->index();
		return tb->index();
	}
	pthread_mutex_lock(&lock);
	tb->__open("device/raw/file.txt");
	int x;
        if (access("device/raw/file.txt", F_OK) == 0 && tb->created){      
        	x = fallocate(tb->fd(), 0, 0, raw_size * 1024 * 1024);     
	}
	if(tb->fd() < 0){
		blobs[tb->index()]->ti->result = -1;
		return -1;
	}
        tb->setOffset(global_dev_offset);
        global_dev_offset += blob_size;
	pthread_mutex_unlock(&lock);
	pthread_barrier_wait(&barrier);
	blobs[tb->index()]->ti->result = tb->index();
	return tb->index();
}

void tb_free_blob(int index){
	char *tmp_data;
        if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))
                std::cout << "posix_memalign failed!" << std::endl;
	if(!mode){
		if(blobs[index]->fd() >= 0)
			if(index >= 0 && index <= Tinyblob::cnt_blob()){ 
				pthread_mutex_lock(&lock);
                        	memcpy(tmp_data, "0", 2);
				pthread_mutex_unlock(&lock);
                        	int x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, 0);
				fallocate(blobs[index]->fd(), FALLOC_FL_PUNCH_HOLE, 0, blob_size);
				blobs[index]->__free = true;
				blobs[index]->__close();
			}
	}
        if(blobs[index]->fd() >= 0)                                                       
                if(index >= 0 && index <= Tinyblob::cnt_blob())                      
                        blobs[index]->__close();
}

int tb_write_blob(int index, void *data){  
	char *tmp_data;
	static uint32_t global_dev_offset = 0;
	if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))
		std::cout << "posix_memalign failed!" << std::endl;
        if(!mode){
		if(blobs[index]->rbw){
			pread(blobs[index]->fd(), tmp_data, blob_size, 0);
			if(tmp_data[0] != '1')
				blobs[index]->rbw = 0;
		}
                if(blobs[index]->fd() >= 0 && !blobs[index]->rbw){       
			memcpy(tmp_data, "1", 2);
                        int x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, 0);
			memcpy(tmp_data, data, blob_size+1);
			x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, ALIGNMENT);
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
			if(!tb->rbw){
                		int x = fallocate(tb->fd(), 0, 0, blob_size);        
                		if(x < 0)                                  
                        		return -1;   
			} 
			int x = tb_write_blob(tb->index(), data);
			return x;
		}
        }
        if(pread(blobs[index]->fd(), tmp_data, blob_size, blobs[index]->offset()+0) == -1)
		std::cout << "read failed!" << std::endl;
	if(tmp_data[0] != '1')
                blobs[index]->rbw = 0;

        if(blobs[index]->fd() >= 0 && !blobs[index]->rbw){
                memcpy(tmp_data, "1", 2);
                int x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, blobs[index]->offset()+0);
                memcpy(tmp_data, data, blob_size+1);     
		x = pwrite(blobs[index]->fd(), (const void *)(tmp_data), blob_size, blobs[index]->offset()+ALIGNMENT);
                free(blobs[index]->__io_buffer);
                if(posix_memalign(&(blobs[index]->__io_buffer), ALIGNMENT, blob_size))
                	std::cout << "posix_memalign failed!" << std::endl;
		tb_flush();
		blobs[index]->__free = false;
                return x;                    
        } 
        else{    
        	Tinyblob *tb = new Tinyblob();
                tb->__open("device/raw/file.txt");      
                tb->setOffset(global_dev_offset);
                global_dev_offset += blob_size;
                blobs.push_back(tb);      
                int x = tb_write_blob(tb->index(), data);
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
                int x = pread(blobs[ti->i]->fd(), tmp_data, blob_size, ALIGNMENT);
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
	//In as2 runs for both modes
	int i;
	for(i = 0; i < Tinyblob::cnt_blob(); i++){
		int fd = blobs[i]->fd();
		if(fd >= 0)
			if(fsync(fd) == 0) 
				return;
			else 
				exit(EXIT_FAILURE);
	}
}

void tb_shutdown(){
	//In as2 also writes everything to device
        for(int i = 0; i < nprocs; i++){
                int x;              
                if((x = tb_write_blob(i, blobs[i]->__io_buffer)) == -1)
                        std::cout << "[SHUTDOWN] *ERROR* : tb_write_blob failed!" << std::endl;
        }

	tb_flush();
}

void tb_init(char *location){
DIR *dir;
struct dirent *ent;
if ((dir = opendir (location)) != NULL) {
  while ((ent = readdir (dir)) != NULL) {
	if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
		continue;
	if(!mode){
        	Tinyblob *tb = new Tinyblob();
		tb->__free = false;
		pthread_mutex_lock(&lock);
        	blobs.push_back(tb);      
		pthread_mutex_unlock(&lock);
		char final_location[30] = "";
		strcat(final_location, location);
		strcat(final_location, ent->d_name);
		tb->__open(final_location);
        	tb->setOffset(0);
	}
	else{
        	char *tmp_data;    
                Tinyblob *tb = new Tinyblob();
		tb->__free = false;
		pthread_mutex_lock(&lock);
                blobs.push_back(tb);      
		pthread_mutex_unlock(&lock);
                char final_location[30] = "";
                strcat(final_location, location);
                strcat(final_location, ent->d_name);
                tb->__open(final_location);
		tb->setOffset(global_dev_offset);
        	if(posix_memalign((void**)&tmp_data, ALIGNMENT, blob_size))                       
                	std::cout << "posix_memalign failed!" << std::endl;  
                if(pread(tb->fd(), tmp_data, blob_size, global_dev_offset+0) == -1)
                        std::cout << "read failed!" << std::endl;
		//Added in as2
                if(pread(tb->fd(), tb->__io_buffer, blob_size, global_dev_offset+ALIGNMENT) == -1)
                        std::cout << "read failed!" << std::endl;
		global_dev_offset += blob_size;
		while(tmp_data[0] == '0' || tmp_data[0] == '1'){
                	Tinyblob *tb = new Tinyblob();
			tb->__free = false;
                	pthread_mutex_lock(&lock);
                	blobs.push_back(tb);      
                	pthread_mutex_unlock(&lock);
			char final_location2[30] = "";
                	strcat(final_location2, location);
                	strcat(final_location2, ent->d_name);
                	tb->__open(final_location2);
			tb->setOffset(global_dev_offset);
			global_dev_offset += blob_size;
                	if(pread(tb->fd(), tmp_data, blob_size, global_dev_offset+0) == -1)
                        	std::cout << "read failed!" << std::endl;
			//Added in as2
                	if(pread(tb->fd(), tb->__io_buffer, blob_size, global_dev_offset+ALIGNMENT) == -1)
                        	std::cout << "read failed!" << std::endl;
		}
        	if(tmp_data[0] != '1')                    
                	tb->rbw = 0;    	
	}
  }
  closedir (dir);
} else {
  perror ("");
  return;
}
}
