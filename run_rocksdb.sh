sudo rm -rf /mnt/datavol
sudo mkdir -p /mnt/datavol

sudo umount /mnt/datavol
sudo umount /mnt/fmap
sudo mdadm --stop /dev/md0
sudo mdadm --zero-superblock /dev/nvme0n1 /dev/nvme1n1 /dev/nvme3n1 /dev/nvme4n1

sudo blkdiscard /dev/nvme0n1
sudo blkdiscard /dev/nvme1n1
sudo blkdiscard /dev/nvme3n1
sudo blkdiscard /dev/nvme4n1

sudo mdadm --create --verbose /dev/md0 --level=stripe --chunk=1024 --raid-devices=4 /dev/nvme0n1 /dev/nvme1n1 /dev/nvme3n1 /dev/nvme4n1

sudo mkfs.ext4 /dev/md0
sudo mount /dev/md0 /mnt/datavol

rm -rf err                                                                                        
rm -rf out                                                          
make                                                                 
sudo ./ycsbc -db rocksdb -threads 1 -P workloads/workloada.spec $1 > err 2>&1 > out
