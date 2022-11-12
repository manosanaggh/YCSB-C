sudo rm -rf /mnt/datavol
sudo mkdir -p /mnt/datavol
rm -rf err                                                                                        
rm -rf out                                                          
make                                                                 
sudo ./ycsbc -db rocksdb -threads 1 -P workloads/workloada.spec $1 > err 2>&1 > out
