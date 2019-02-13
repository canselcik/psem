#include "ruby.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>


VALUE psem_init(VALUE self, VALUE r_sem_name)
{
    // TODO: Needs to start with '/'
    Check_Type(r_sem_name, T_STRING);
    const char* sem_name = StringValueCStr(r_sem_name);

    sem_t* sem = sem_open(sem_name, O_CREAT, strtol("0644", 0, 8), 0);

    rb_ivar_set(self, rb_intern("ptr"), (unsigned long)sem);
    return self;
}

VALUE psem_get(VALUE self)
{
    sem_t* sem = (sem_t*)rb_ivar_get(self, rb_intern("ptr"));
    printf("sem_open(...) returned: %p\n", sem);

    return INT2NUM(1);
}

void Init_psem()
{
    VALUE PSem = rb_define_class("PSem", rb_cObject);
    rb_define_method(PSem, "initialize", psem_init, 1);
    rb_define_method(PSem, "ok?", psem_ok, 0);
}
