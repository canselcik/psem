#include "ruby.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>


VALUE psem_load(VALUE self, VALUE r_sem_name)
{
    const char* sem_name = StringValueCStr(r_sem_name);
    sem_t* sem = sem_open(sem_name, O_CREAT);
    printf("GOT %p\n", sem);
    return rb_str_new2("ok");
}

void Init_psem()
{
    VALUE PSem = rb_define_class("PSem", rb_cObject);
    rb_define_method(PSem, "load", psem_load, 1);
}
