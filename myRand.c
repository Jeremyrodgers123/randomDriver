

//Should be all that's needed
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h> //device stuff
#include <linux/cdev.h> 
#include <linux/uaccess.h>  //for put_user, get_user, etc
#include <linux/slab.h>  //for kmalloc/kfree
#include <linux/fs.h>

MODULE_AUTHOR("Ben Jones + MSD");
MODULE_LICENSE("GPL");  //the kernel cares a lot whether modules are open source

//basically the file name that will be created in /dev
#define MY_DEVICE_NAME "myRand"

//Kernel stuff for keeping track of the device
static unsigned int myRand_major = 0;
static struct class *myRand_class = 0;
static struct cdev cdev; //the device
static spinlock_t my_lock = __SPIN_LOCK_UNLOCKED();

/*

DEFINE ALL YOUR RC4 STUFF HERE

 */
struct RC4{
  uint8_t arr[256];
  int size;
  int i;
  int j;
};

struct RC4 rc4;

void createIdentityArr( const int size, uint8_t *arr){
  int i;
  for(i = 0; i < size; i++){
    arr[i] = i;
  }
}

void createKey( char passcode [], uint8_t key [],int keyLen){
  int i, index;
  //passcodeLen = stringLen(passcode);
  for(i = 0; i < keyLen; i++){
    index = i % 8;
    key[ index ] = key[ index ] ^ passcode[i];
  }
}

void swapIt(int a, int b){
  uint8_t temp = rc4.arr[a];
  rc4.arr[a] = rc4.arr[b];
  rc4.arr[b] = temp;
}

void initKey( uint8_t *key, int length){
  int i = 0;
  for(i = 0; i < length; i++){
    key[i] = 0;
  }
}

void myPrint(unsigned char *passcode, int length){
  int i;
  for( i =0; i < length; i++){
    printk("%d: %u", i, passcode[i]);
  }
}

void initRC4(unsigned char *passcode, int keyLength){
  int i, j;
  printk("InitRC4:1 starting initRC4");
  //printk("passcode: %u", passcode[0]);
  rc4.size = 256;
  rc4.i = 0;
  rc4.j = 0;
  createIdentityArr(256, rc4.arr);
  uint8_t key[keyLength];
  initKey(key, keyLength);// make variable len key
  createKey(passcode, key, keyLength);
  //printk("key created: ");
  //myPrint(key, keyLength);
  j = 0;
  for(i = 0; i < rc4.size; i++){
    j = (j + rc4.arr[i] + key [i % 8 ]) % rc4.size;
    swapIt(i, j);
  }
  //printk("RC4Init: Done");
  //  printk("printState");
  //for(i = 0; i < rc4.size; i++){
  // printk("%d", rc4.arr[i]);
  //}
}

unsigned char rc4Next(void){
  //printk("rc4Next:start");
  int  k, t;
  k = t = 0;
  rc4.i = (rc4.i + 1) % 256;
  rc4.j = (rc4.j + 1) % 256;
  swapIt(rc4.i, rc4.j);
  t = (rc4.arr[rc4.i] + rc4.arr[rc4.j]) % 256;
  // printk("rc4Next:T:%d", t);
  return t;
}

void getRandomBytes(char *buffer, int count){
  int i;
  int max;
  max = 0;
  printk("getRandomBytes:count: %u", count);
  for(i = 0; i < count; i++){
      buffer[i] = rc4Next();
      if(i > max){
	max = i;
      }
    }
  printk("getRandomBytes: finished");
  printk("max %d", max);
}

void copyToUsr(char *src, char *dest, int length){
  int i;
  for( i = 0; i < length; i++){
    put_user(src[i], &dest[i]);
  }
}
/*
  called when opening a device.  We won't do anything
 */
int myRand_open(struct inode *inode, struct file *filp){
  return 0; //nothing to do here
}

/* called when closing a device, we won't do anything */
int myRand_release(struct inode *inode, struct file *filp){
  return 0; //nothing to do here
}

ssize_t myRand_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){

  /* 
	 FILL THE USER'S BUFFER WITH BYTES FROM YOUR RC4 GENERATOR 
	 BE SURE NOT TO DIRECTLY DEREFERENCE A USER POINTER!

*/
  int retVal;
  printk("\n");
  printk("1 Read");
  //allocate empty buffer
  char *userBuffer = kmalloc( (sizeof(char) * count), GFP_KERNEL);
  if(userBuffer == NULL){
    printk("error occured");
    return -1;
  }
  //printk("2 Read: buffer size: %zu", count);
  spin_lock(&my_lock);
  //write bytes to buffer
  getRandomBytes(userBuffer, count);
  printk("Read: have random Bytes");
  //myPrint(userBuffer, count);
 
  //copy to userbuffer
  printk("start copy buffer");
  //retVal = __copy_to_user( buf, userBuffer, count);
  //put_user(1, buf);
  //put_user(2, &buf[1]);
  //put_user(3, &buf[2]);
  //put_user(4, &buf[3]);
  copyToUsr(userBuffer, buf, count);
  printk("wrote to user");
  spin_unlock(&my_lock);
  //printk("make it out of the lock");
  //if(retVal != 0){
  //printk("there was an error with copy");
  //printk( "%d", retVal);
  //}
  printk("copied user buffer");
  //free bytes
  kfree(userBuffer);
  return count;
}

ssize_t myRand_write(struct file*filp, const char __user *buf, size_t count, loff_t *fpos){
  /*
	USE THE USER's BUFFER TO RE-INITIALIZE YOUR RC4 GENERATOR
	BE SURE NOT TO DIRECTLY DEREFERENCE A USER POINTER!
   */
  //allocate empty buffer
  char *userBuffer = kmalloc( (sizeof(char) * count), GFP_KERNEL);
  if(userBuffer == NULL){
    printk("buffer is NULL");
    return -1;
  }
  printk("i: write:: count: %zu", count);
  //copy bytes to kernel buffer
  copy_from_user(userBuffer, buf, count);
  spin_lock(&my_lock);
  //reset state with new key
  initRC4(userBuffer, count);
  printk("i:write:**** REINIT*****");
  printk("\n");
  myPrint(userBuffer, count);
  //fill kernel buffer
  //getRandomBytes(userBuffer, count);
  spin_unlock(&my_lock);
  kfree(userBuffer);
  return count;
}

/* respond to seek() syscalls... by ignoring them */
loff_t myRand_llseek(struct file *rilp, loff_t off, int whence){
  return 0; //ignore seeks
}

/* register these functions with the kernel so it knows to call them in response to
   read, write, open, close, seek, etc */
struct file_operations myRand_fops = {
  .owner = THIS_MODULE,
  .read = myRand_read,
  .write = myRand_write,
  .open = myRand_open,
  .release = myRand_release,
  .llseek = myRand_llseek
};

/* this function makes it so that this device is readable/writable by normal users.
   Without this, only root can read/write this by default */
static int myRand_uevent(struct device* dev, struct kobj_uevent_env *env){
  add_uevent_var(env, "DEVMODE=%#o", 0666);
  return 0; 
}

/* Called when the module is loaded.  Do all our initialization stuff here */
static int __init
myRand_init_module(void){
  dev_t dev;
  dev_t devno;
  int err;
  int minor;

  /*
	INITIALIZE YOUR RC4 GENERATOR WITH A SINGLE 0 BYTE
   */
  uint8_t arr[1];
  arr[0] = 0;
  printk("\n");
  printk("\n");
  printk("1 MAIN INIT!!!!");
  //spin_lock(&my_lock);
  initRC4(arr, 1);
  //spin_unlock(&my_lock);

  /*  This allocates necessary kernel data structures and plumbs everything together */
  dev = 0;
  err = 0;
  err = alloc_chrdev_region(&dev, 0, 1, MY_DEVICE_NAME);
  if(err < 0){
    printk(KERN_WARNING "[target] alloc_chrdev_region() failed\n");
  }
  myRand_major = MAJOR(dev);
  myRand_class = class_create(THIS_MODULE, MY_DEVICE_NAME);
  if(IS_ERR(myRand_class)) {
    err = PTR_ERR(myRand_class);
    goto fail;
  }

  /* this code uses the uevent function above to make our device user readable */
  myRand_class->dev_uevent = myRand_uevent;
  minor = 0;
  devno = MKDEV(myRand_major, minor);
  struct device *device = NULL;

  cdev_init(&cdev, &myRand_fops);
  cdev.owner = THIS_MODULE;

  err = cdev_add(&cdev, devno, 1);
  if(err){
    printk(KERN_WARNING "[target] Error trying to add device: %d", err);
    return err;
  }
  device = device_create(myRand_class, NULL, devno, NULL, MY_DEVICE_NAME);

  if(IS_ERR(device)) {
    err = PTR_ERR(device);
    printk(KERN_WARNING "[target error while creating device: %d", err);
    cdev_del(&cdev); //clean up dev
    return err;
  }
  printk("module loaded successfully\n");
  return 0;

 fail:
  printk("something bad happened!\n");
  return -1;
}

/* This is called when our module is unloaded */
static void __exit
myRand_exit_module(void){
  device_destroy(myRand_class, MKDEV(myRand_major, 0));
  cdev_del(&cdev);
  if(myRand_class){
    class_destroy(myRand_class);
  }
  unregister_chrdev_region(MKDEV(myRand_major, 0), 1);
  printk("Unloaded my random module");
}

module_init(myRand_init_module);
module_exit(myRand_exit_module);
