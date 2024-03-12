dd if=/dev/zero of=disk.img bs=1M count=128
sudo losetup /dev/loop32 disk.img
sudo gparted -H /dev/loop32
sudo losetup -d /dev/loop32
