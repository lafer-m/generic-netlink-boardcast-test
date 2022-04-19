#include<linux/cdev.h>
#define KEY_SIZE 64
typedef struct
{
    char  key[KEY_SIZE+1];
    atomic_t available;
    atomic_t key_available;
    struct cdev cdev;
    struct class *dev_class;
} dacs_dev;

int dacs_dev_init(void);
void dacs_dev_uninit(void);
dacs_dev * get_dacs_dev(void);