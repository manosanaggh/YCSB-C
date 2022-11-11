rm -rf err                                                                                        
rm -rf out                                                          
make                                                                 
./ycsbc -db rocksdb -threads 1 -P workloads/workloada.spec $1 > err 2>&1 > out
