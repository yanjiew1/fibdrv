#include <linux/bitops.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Fibonacci engine driver");
MODULE_VERSION("0.1");

#define DEV_FIBONACCI_NAME "fibonacci"

/* MAX_LENGTH is set to 92 because
 * ssize_t can't fit the number > 92
 */
#define MAX_LENGTH 92

static dev_t fib_dev = 0;
static struct cdev *fib_cdev;
static struct class *fib_class;

struct fibdrv_priv {
    size_t size;
    size_t pos;
    char *result;
    int impl;
    struct mutex lock;
    ktime_t start;
    ktime_t end;
};

static int fib_num_of_u64(unsigned int k)
{
    int digits = ((long) k * 1084753 - 18140062) / (10 * 9) + 1;
    digits = digits < 1 ? 1 : digits;
    return digits;
}

/* Fibonacci Sequence using dynamic programming */
static int fib_sequence_dp(unsigned int k, struct fibdrv_priv *priv)
{
    long long f[2];

    f[0] = 0;
    f[1] = 1;

    for (int i = 2; i <= k; i++) {
        f[i % 2] = f[0] + f[1];
    }

    priv->result = kmalloc(sizeof(long long), GFP_KERNEL);
    if (!priv->result)
        return -ENOMEM;

    priv->size = sizeof(long long);
    memcpy(priv->result, &f[k % 2], sizeof(long long));

    return 0;
}

/* Fibonacci Sequence using fast doubling */
static int fib_sequence_fast_doubling(unsigned int k, struct fibdrv_priv *priv)
{
    long long fib[2] = {0, 1};
    int len = fls(k);
    k <<= 32 - len;

    for (; len; len--, k <<= 1) {
        /* Fast doubling */
        long long tmp = fib[0];
        fib[0] = fib[0] * (2 * fib[1] - fib[0]);
        fib[1] = fib[1] * fib[1] + tmp * tmp;

        if (k & (1U << 31)) {
            /* Fast doubling + 1 */
            tmp = fib[0];
            fib[0] = fib[1];
            fib[1] = fib[1] + tmp;
        }
    }

    priv->result = kmalloc(sizeof(long long), GFP_KERNEL);
    if (!priv->result)
        return -ENOMEM;

    priv->size = sizeof(long long);
    memcpy(priv->result, &fib[0], sizeof(long long));

    return 0;
}

/* Fibonacci Sequence using big number */
static long long fib_sequence(int k, struct fibdrv_priv *priv)
{
    switch (priv->impl) {
    case 2:
        return fib_sequence_dp(k, priv);
    default:
        return fib_sequence_fast_doubling(k, priv);
    }
}

static void fib_init_priv(struct fibdrv_priv *priv)
{
    priv->size = 0;
    priv->result = NULL;
    priv->impl = 0;
    priv->start = 0;
    priv->end = 0;
    mutex_init(&priv->lock);
}

static void fib_free_priv(struct fibdrv_priv *priv)
{
    kfree(priv->result);
    mutex_destroy(&priv->lock);
    kfree(priv);
}

static int fib_open(struct inode *inode, struct file *file)
{
    struct fibdrv_priv *priv = kmalloc(sizeof(struct fibdrv_priv), GFP_KERNEL);
    if (!priv) {
        printk(KERN_ALERT "kmalloc failed");
        return -ENOMEM;
    }
    fib_init_priv(priv);
    file->private_data = priv;
    return 0;
}

static int fib_release(struct inode *inode, struct file *file)
{
    fib_free_priv((struct fibdrv_priv *) file->private_data);
    return 0;
}

/* calculate the fibonacci number at given offset */
static ssize_t fib_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
    struct fibdrv_priv *priv = (struct fibdrv_priv *) file->private_data;
    ssize_t ret = 0;

    mutex_lock(&priv->lock);

    /* Whether we need to calculate new value */
    if (!priv->result) {
        ktime_t start, end;
        start = ktime_get();
        fib_sequence((int) *offset, priv);
        end = ktime_get();
        priv->start = start;
        priv->end = end;
    }

    if (size) {
        /* Copy to user */
        char *bufread = priv->result + priv->pos;
        int release = 0;
        if (priv->size - priv->pos < size) {
            /* The returned data is less than the data copied.
             * The user space program should know that it read all the data.
             */
            release = 1;
            size = priv->size - priv->pos;
        }
        ret = size;

        if (copy_to_user(buf, bufread, size) != size)
            ret = -EFAULT;
        else
            priv->pos += size;

        if (release) {
            /* Discard the result */
            kfree(priv->result);
            priv->result = NULL;
        }
    }
    mutex_unlock(&priv->lock);
    return ret;
}

/* write operation is skipped */
static ssize_t fib_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{
    struct fibdrv_priv *priv = (struct fibdrv_priv *) file->private_data;
    ssize_t ret = 0;
    mutex_lock(&priv->lock);
    if (size == 0)
        ret = priv->start;
    else if (size == 1)
        ret = priv->end;
    else
        priv->impl = (int) size;
    mutex_unlock(&priv->lock);
    return ret;
}

static loff_t fib_device_lseek(struct file *file, loff_t offset, int orig)
{
    struct fibdrv_priv *priv = (struct fibdrv_priv *) file->private_data;
    loff_t new_pos = 0;
    mutex_lock(&priv->lock);
    kfree(priv->result);
    priv->result = NULL;

    switch (orig) {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    mutex_unlock(&priv->lock);
    return new_pos;
}

const struct file_operations fib_fops = {
    .owner = THIS_MODULE,
    .read = fib_read,
    .write = fib_write,
    .open = fib_open,
    .release = fib_release,
    .llseek = fib_device_lseek,
};

static int __init init_fib_dev(void)
{
    int rc = 0;

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&fib_dev, 0, 1, DEV_FIBONACCI_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the fibonacci char device. rc = %i",
               rc);
        return rc;
    }

    fib_cdev = cdev_alloc();
    if (fib_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    fib_cdev->ops = &fib_fops;
    rc = cdev_add(fib_cdev, fib_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    fib_class = class_create(THIS_MODULE, DEV_FIBONACCI_NAME);

    if (!fib_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(fib_class, NULL, fib_dev, NULL, DEV_FIBONACCI_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(fib_class);
failed_class_create:
    cdev_del(fib_cdev);
failed_cdev:
    unregister_chrdev_region(fib_dev, 1);
    return rc;
}

static void __exit exit_fib_dev(void)
{
    device_destroy(fib_class, fib_dev);
    class_destroy(fib_class);
    cdev_del(fib_cdev);
    unregister_chrdev_region(fib_dev, 1);
}

module_init(init_fib_dev);
module_exit(exit_fib_dev);
