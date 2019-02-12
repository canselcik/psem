#include "ruby.h"

VALUE working(VALUE self)
{
    return rb_str_new2("ok");
}

void Init_psem()
{
    VALUE PSem = rb_define_class("PSem", rb_cObject);
    rb_define_method(PSem, "working", working, 0);
}
