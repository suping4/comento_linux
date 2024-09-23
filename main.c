#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/spinlock.h>

#define BUF_SIZE 16
#define COMENTO_IOCTL_CLEAR _IO('c', 0)

static DEFINE_RWLOCK(lock);
static char comento_buf[BUF_SIZE] = {0, };

static ssize_t comento_device_read(struct file *file, char __user *buf, size_t len, loff_t *ppos) {
       ssize_t ret;
	read_lock(&lock);
	if (BUF_SIZE <= len + *ppos) {
		len = BUF_SIZE - *ppos;
	}
	ret = len - copy_to_user(buf, comento_buf + *ppos, len);
	*ppos += ret;
	read_unlock(&lock);
	return ret;
}

static ssize_t comento_device_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos) {
       ssize_t ret;
        write_lock(&lock);
        if (BUF_SIZE <= len + *ppos) {
                len = BUF_SIZE - *ppos;
        }
        ret = len - copy_from_user(comento_buf + *ppos, buf, len);
        *ppos += ret;
        write_unlock(&lock);
        return ret;
}


static int comento_device_open(struct inode *inode, struct file *file) {
	int minor = iminor(inode);
	printk(KERN_DEBUG "%s - minor : %d\n", __func__, minor);
	return 0;
}

static long comento_device_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
	switch (cmd){
	case COMENTO_IOCTL_CLEAR:
		memset(comento_buf, 0, BUF_SIZE);
		break;
	default:
		printk(KERN_DEBUG "%s failed - %d\n", __func__, cmd);
		return -EINVAL;
	}
	return 0;
}

struct file_operations fops = {
	.open = comento_device_open,
	.read = comento_device_read,
	.write = comento_device_write,
	.unlocked_ioctl = comento_device_ioctl,
};

static struct class *class;
static struct device *device;

static int __init comento_module_init(void)
{
	printk(KERN_DEBUG "%s\n", __func__);
	int major = register_chrdev(0, "comento", &fops);
	int minor = 17;
	class = class_create("comento");
	device = device_create(class, NULL, MKDEV(major, 17), NULL, "%s%d", "comento", minor);
	return 0;
}

static void __exit comento_module_exit(void)
{
	unregister_chrdev(177, "comento");
        printk(KERN_DEBUG "%s\n", __func__);
}


module_init(comento_module_init);
module_exit(comento_module_exit);

MODULE_AUTHOR("Hello<hello@comento.com");
MODULE_DESCRIPTION("Example module");
MODULE_LICENSE("GPL v2");
