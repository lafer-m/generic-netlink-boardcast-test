#include <linux/cdev.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/cdev.h>	
#include <linux/uaccess.h>
#include "cudev.h"


int dacs_dev_major = 0;
int dacs_dev_minor = 0;

int dacs_dev_open(struct inode *inode, struct file *filp);
int dacs_dev_release(struct inode *inode, struct file *filp);
long dacs_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#define DEVICE_NAME "dacs_d"
#define WR_VALUE _IOW('a', 'a', unsigned long)
char *dacs_dev_name = DEVICE_NAME;
char *dacs_dev_class_name = "dacs_d_class";

dacs_dev dacs_dev_interface;

struct file_operations dacs_dev_interface_fops = {
    .owner = THIS_MODULE,
    .read = NULL,
    .write = NULL,
    .open = dacs_dev_open,
    .unlocked_ioctl = dacs_dev_ioctl,
    .release = dacs_dev_release,
};


int dacs_dev_open(struct inode *inode, struct file *filp) {
    dacs_dev *dacs_dev_interface;
    pr_info("callback open");
    dacs_dev_interface = container_of(inode->i_cdev, dacs_dev, cdev);
    filp->private_data = dacs_dev_interface;
    if (!atomic_dec_and_test(&dacs_dev_interface->available)) {
        pr_err("dacs_d device already opened\n");
        atomic_inc(&dacs_dev_interface->available);
        return -EBUSY;
    }
    return 0;
};

int dacs_dev_release(struct inode *inode, struct file *filp) {
    dacs_dev *dacs_dev_interface = filp->private_data;
    atomic_inc(&dacs_dev_interface->available);
    return 0;
};

long dacs_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    pr_info("ioctl callback");
    switch (cmd)
    {
    case /* constant-expression */WR_VALUE:
        /* code */

        pr_info("set the share key");
        dacs_dev *dacs_dev_interface = filp->private_data;
        if (copy_from_user(&dacs_dev_interface->key, (char *) arg, KEY_SIZE)) {
            pr_err("data write err\n");
            break;
        }
        dacs_dev_interface->key[KEY_SIZE] = '\0';
        pr_info("key is: %s\n", dacs_dev_interface->key);
        int ret = atomic_read(&dacs_dev_interface->key_available);
        if (ret == 0) {
            atomic_inc(&dacs_dev_interface->key_available);
        }
        break;
    
    default:
        break;
    }
    return 0;
};


int __init dacs_dev_init(void) {
    pr_info("start to create dacs_d device\n");
    dev_t  devno = 0;
    int ret;
    // init dacs_dev_interface
    memset(&dacs_dev_interface, 0, sizeof(dacs_dev));
    atomic_set(&dacs_dev_interface.available, 1);
    atomic_set(&dacs_dev_interface.key_available, 0);

    ret = alloc_chrdev_region(&devno, dacs_dev_minor, 1, dacs_dev_name);
    dacs_dev_major = MAJOR(devno);
    if (ret < 0) {
        pr_err("alloc dev region err: %i\n", ret);
        goto error;
    }

    dev_t devn = MKDEV(dacs_dev_major, dacs_dev_minor);
    cdev_init(&dacs_dev_interface.cdev, &dacs_dev_interface_fops);
    // dacs_dev_interface.cdev.owner = THIS_MODULE;
    // dacs_dev_interface.cdev.ops = &dacs_dev_interface_fops;

    ret = cdev_add(&dacs_dev_interface.cdev, devn, 1);
    if (ret < 0) {
        pr_err("add dev to system err: %i\n", ret);
        goto error;
    }
    
    struct class *dev_class = class_create(THIS_MODULE, dacs_dev_class_name);
    if (dev_class == NULL) {
        pr_err("cannot create the class\n");
        goto error;
    }
    dacs_dev_interface.dev_class = dev_class;
    if ((device_create(dacs_dev_interface.dev_class, NULL, devno,NULL, dacs_dev_name)) == NULL) {
        pr_err("cannot create the device\n");
        goto error;
    }

    pr_info("created dacs_d device ok\n");
    return 0;
error:
    dacs_dev_uninit();
    return ret;
};

void __exit dacs_dev_uninit(void) {
    dev_t devno = MKDEV(dacs_dev_major, dacs_dev_minor);
    device_destroy(dacs_dev_interface.dev_class, devno);
    class_destroy(dacs_dev_interface.dev_class);
    cdev_del(&dacs_dev_interface.cdev);
    unregister_chrdev_region(devno, 1);
    pr_info("dacs_d device unloaded\n");
};

dacs_dev * get_dacs_dev(void) {
    return &dacs_dev_interface;
};