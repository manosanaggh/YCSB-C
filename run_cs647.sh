rm -rf device
mkdir -p device
mkdir -p device/blobs
mkdir -p device/raw
rm -rf err
rm -rf out
make
./ycsbc -db cs647 -threads 1 -P workloads/workloada.spec $1 > err 2>&1 > out
