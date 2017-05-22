# referenced some test parameters from https://gist.github.com/iankronquist/c3f3ef5dcec8d812a359ce6a723c2f05

echo "Starting test.\n"

echo "Setting up module, creating filesystem, and mounting."

insmod ramdisk.ko
shred -z /dev/sbd0
mkfs.ext2 /dev/sbd0
lsmod
mount /dev/sbd0 /mnt/test

echo "Writing 'Testing homework 3!' to a file on mounted disk."

echo "Testing homework 3!" > /mnt/test/test.txt

echo "Wrote to file."

echo "Try to search for 'Testing homework 3!' on /dev/sbd0. Nothing should appear."

grep -a "Testing homework 3!" /dev/sbd0

echo "Result of trying to view file on /mnt/test using 'cat'"

cat /mnt/test/test.txt

echo "Trying to list contents of /dev/sbd0 using cat should output garbage."

cat /dev/sbd0

echo "Umounting and removing module."

umount /mnt/test
rmmod ramdisk.ko
