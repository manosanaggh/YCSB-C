: 'sudo umount /mnt/fmap
sudo umount /mnt/datavol
sudo mdadm --stop /dev/md0
sudo mdadm --zero-superblock /dev/nvme0n1 /dev/nvme1n1 /dev/nvme3n1 /dev/nvme4n1

sudo blkdiscard /dev/nvme0n1
sudo blkdiscard /dev/nvme1n1
sudo blkdiscard /dev/nvme3n1
sudo blkdiscard /dev/nvme4n1

sudo mdadm --create --verbose /dev/md0 --level=stripe --chunk=1024 --raid-devices=4 /dev/nvme0n1 /dev/nvme1n1 /dev/nvme3n1 /dev/nvme4n1

sudo mkfs.ext4 /dev/md0
sudo mount /dev/md0 /mnt/fmap
'
DEV_PATH_PREF=/mnt/fmap/device/

sudo rm -rf $DEV_PATH_PREF
sudo mkdir -p $DEV_PATH_PREF
sudo mkdir -p $DEV_PATH_PREF/blobs
sudo mkdir -p $DEV_PATH_PREF/raw
rm -rf err
rm -rf out
make
sudo ./ycsbc -db cs647 -threads 1 -P workloads/workloadf.spec $1 > err 2>&1 > out
