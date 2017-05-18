/*
 * A sample, extra-simple block driver. Updated for kernel 2.6.31.
 *
 * (C) 2003 Eklektix, Inc.
 * (C) 2010 Pat Patterson <pat at superpat dot com>
 * Redistributable under the terms of the GNU GPL.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/crypto.h>

MODULE_LICENSE("Dual BSD/GPL");
static char *Version = "1.4";

static int major_num = 0;
module_param(major_num, int, 0);
static int logical_block_size = 512;
module_param(logical_block_size, int, 0);
static int nsectors = 1024; /* How big the drive is */
module_param(nsectors, int, 0600);

//Crypto stuff
#define KEY_SIZE 32 /* AES has a maximum key size of 256 bits */
static char crypto_key[KEY_SIZE];
static int key_size = 0; /* size of the current key */
/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE 512

/*
 * Our request queue.
 */
static struct request_queue *Queue;

//Base cipher struct
/*
 * The internal representation of our device.
 */
static struct sbd_device {
	unsigned long size;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
} Device;

struct crypto_cipher *tfm;

//ssize_t: This data type is used to represent the 
//sizes of blocks that can be read or written in a
// single operation. It is similar to size_t , but 
// must be a signed type.

ssize_t key_display(struct device *dev, struct device_attribute *attr, char *buf)
{	
	//Printing out we are copying the key
	//Formating the crypto key string and placing 
	//it into a buffer with scnprintf
	printk(KERN_DEBUG "sbd: copying the key\n");
	return scnprintf(buf, PAGE_SIZE, "%s\n", crypto_key);
}

//Stores the key into the buffer
//first it checks that the count size is not invalid
//then formats the key and puts it into the buffer
ssize_t key_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){
	if(count != 16 && count != 24 && count != 32){
		printk(KERN_WARNING "sbd: error, key size is invalid %lu\n", (unsigned long)count);
		return -EINVAL;
	}
	printk(KERN_DEBUG "sbd: storing the cipher key \n");
	//formats the crypto key into a printf format and stores 
	//the key in the buffer location pointed to by crypto_key
	snprintf(crypto_key, sizeof(crypto_key), "%.*s", (int)min(count, sizeof(crypto_key) - 1),buf);
	key_size = count;
	return count;
}

DEVICE_ATTR(key, 0600, key_display, key_store);

static void rd_root_dev_release(struct device *dev)
{
}

// sysfs
static struct device rd_root_dev = {
	.init_name = "sbd",
	.release = rd_root_dev_release,
};

/*
 * Handle an I/O request.
 */
static void sbd_transfer(struct sbd_device *dev, sector_t sector,
		unsigned long nsect, char *buffer, int write) {
	unsigned long offset = sector * logical_block_size;
	unsigned long nbytes = nsect * logical_block_size;
	
	int k;

	if(key_size == 0){
		printk(KERN_INFO "no key set\n");
	}else{
		crypto_cipher_clear_flags(tfm, ~0);
		crypto_cipher_setkey(tfm, crypto_key, key_size);
	}
	
	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "sbd: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	//We gonna do some shit here
	//Check the key size is not 0
	///loop through while k is not the size nbytes
	//encrypt one data entry with the offset of the size
	//apply it to the buffer with that size as the offset
	if (write)
	{
		if(key_size != 0)
		{
			for(k=0; k < nbytes; k+= crypto_cipher_blocksize(tfm))
			{
				crypto_cipher_encrypt_one(tfm, dev->data+offset+k, buffer+k);
			}
		}
		else
		{
			memcpy(dev->data + offset, buffer, nbytes);
		}
	}
	else
	{
		if(key_size != 0)
		{
			for (k = 0; k < nbytes; k+= crypto_cipher_blocksize(tfm)) 
			{
				crypto_cipher_decrypt_one(tfm, buffer+k, dev->data+offset+k);
			}
		}
		else
		{
			memcpy(buffer, dev->data + offset, nbytes);
		}
	}
}

static void sbd_request(struct request_queue *q) {
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		// blk_fs_request() was removed in 2.6.36 - many thanks to
		// Christian Paro for the heads up and fix...
		//if (!blk_fs_request(req)) {
		if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
			printk (KERN_NOTICE "Skip non-CMD request\n");
			__blk_end_request_all(req, -EIO);
			continue;
		}
		sbd_transfer(&Device, blk_rq_pos(req), blk_rq_cur_sectors(req),
				req->buffer, rq_data_dir(req));
		if ( ! __blk_end_request_cur(req, 0) ) {
			req = blk_fetch_request(q);
		}
	}
}

/*
 * The HDIO_GETGEO ioctl is handled in blkdev_ioctl(), which
 * calls this. We need to implement getgeo, since we can't
 * use tools such as fdisk to partition the drive otherwise.
 */
int sbd_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
	long size;

	/* We have no real geometry, of course, so make something up. */
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}

/*
 * The device operations structure.
 */
static struct block_device_operations sbd_ops = {
		.owner  = THIS_MODULE,
		.getgeo = sbd_getgeo
};

static int __init sbd_init(void) {
	int ret;
	
	tfm = crypto_alloc_cipher("aes",0,16);

	if (IS_ERR(tfm)){
		printk(KERN_ERR "cipher: transformation failed");
		return PTR_ERR(tfm);
	}	

	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL) {
		return -ENOMEM;
	}

	/*
	 * Set up our internal device.
	 */
	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL)
		return -ENOMEM;
	/*
	 * Get a request queue.
	 */
	Queue = blk_init_queue(sbd_request, &Device.lock);
	if (Queue == NULL)
		goto out;
	blk_queue_logical_block_size(Queue, logical_block_size);
	/*
	 * Get registered.
	 */
	major_num = register_blkdev(major_num, "sbd");
	if (major_num < 0) {
		printk(KERN_WARNING "sbd: unable to get major number\n");
		goto out;
	}
	/*
	 * And the gendisk structure.
	 */
	Device.gd = alloc_disk(16);
	if (!Device.gd)
		goto out_unregister;
	Device.gd->major = major_num;
	Device.gd->first_minor = 0;
	Device.gd->fops = &sbd_ops;
	Device.gd->private_data = &Device;
	strcpy(Device.gd->disk_name, "sbd0");
	set_capacity(Device.gd, nsectors);
	Device.gd->queue = Queue;
	add_disk(Device.gd);
	
	ret = device_register(&rd_root_dev);
	if (ret < 0){
		goto out_unregister;
	}

	ret = device_create_file(&rd_root_dev, &dev_attr_key);
	if (ret < 0) {
		device_unregister(&rd_root_dev);
		goto out_unregister;
	}

	return 0;

out_unregister:
	unregister_blkdev(major_num, "sbd");
out:
	vfree(Device.data);
	crypto_free_cipher(tfm);
	return -ENOMEM;
}

static void __exit sbd_exit(void)
{
	del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(major_num, "sbd");
	blk_cleanup_queue(Queue);
	crypto_free_cipher(tfm);	
	device_remove_file(&rd_root_dev, &dev_attr_key);
	device_unregister(&rd_root_dev);	


	vfree(Device.data);
}

module_init(sbd_init);
module_exit(sbd_exit);
