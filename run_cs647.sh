#sudo rm -rf /mnt/fmap/device
sudo mkdir -p /mnt/fmap/device
sudo mkdir -p /mnt/fmap/device/blobs
sudo mkdir -p /mnt/fmap/device/raw
rm -rf err
rm -rf out
make
sudo ./ycsbc -db cs647 -threads 1 -P workloads/workloada.spec $1 > err 2>&1 > out
