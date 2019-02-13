#include "ruby.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>

static inline sem_t* get_current_semaphore(VALUE self) {
  sem_t* sem = (sem_t*)rb_ivar_get(self, rb_intern("inner"));
  if (sem == NULL) {
      rb_raise(rb_eRuntimeError, "inner semaphore is not initialized");
  }
  return sem;
}

#ifdef _WIN32
   #error Windows not supported
#elif __APPLE__
// OSX has this deprecated and removed
VALUE psem_waitmillis(VALUE self, VALUE rbNum_timeoutMillis) {
    rb_raise(rb_eRuntimeError, "sem_timedwait() is deprecated on OSX");
}
// they also broke sem_getvalue to return 0 no matter what on a path
// to deprecation
VALUE psem_get_value(VALUE self) {
    rb_raise(rb_eRuntimeError, "sem_getvalue() is deprecated on OSX");
}
#else
VALUE psem_waitmillis(VALUE self, VALUE rbNum_timeoutMillis) {
  Check_Type(rbNum_timeoutMillis, T_FIXNUM);

  int ms = FIX2INT(rbNum_timeoutMillis);
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;

  sem_t* sem = get_current_semaphore(self);
  if (sem_timedwait(sem, &ts) == -1) {
      int err = errno;
      switch (err) {
          case ETIMEDOUT:
              return Qfalse;
          case EINTR:
              rb_raise(rb_eRuntimeError, "got interrupted by a signal handler (EINTR)");
              break;
          case EINVAL:
              rb_raise(rb_eRuntimeError, "inner semaphore is not a valid semaphore (EINVAL)");
              break;
          default:
              rb_raise(rb_eRuntimeError, "unknown errno: %d", err);
      }
  }
  return Qtrue;
}

VALUE psem_get_value(VALUE self) {
  sem_t* sem = get_current_semaphore(self);
  int val = 0xFFFF;
  if (sem_getvalue(sem, &val) == -1) {
      rb_raise(rb_eRuntimeError, "inner semaphore is not a valid semaphore, failed with errno: %d", errno);
  }
  return INT2NUM(val);
}
#endif

VALUE psem_unlink(VALUE self, VALUE rbStr_semName) {
    Check_Type(rbStr_semName, T_STRING);

    // Check length and format of semaphore name
    const char* sem_name = StringValueCStr(rbStr_semName);
    if (sem_name == NULL || strlen(sem_name) < 2 || sem_name[0] != '/') {
        rb_raise(rb_eRuntimeError, "invalid name. has to be in the format of '/foobar'");
    }
    if (strlen(sem_name) > 253) {
        rb_raise(rb_eRuntimeError, "name cannot be longer than 253 bytes long");
    }

    // Unlink the semaphore
    if (sem_unlink(sem_name) == -1) {
        switch (errno) {
            case EACCES:
                rb_raise(rb_eRuntimeError, "you do not have permission to unlink this semaphore");
                break;
            case ENAMETOOLONG:
                rb_raise(rb_eRuntimeError, "name was too long");
                break;
            case ENOENT:
                rb_raise(rb_eRuntimeError, "no semaphore matching the name");
                break;
            default:
                rb_raise(rb_eRuntimeError, "unknown error");
        }
    }
    return Qtrue;
}

VALUE psem_exists(VALUE self, VALUE rbStr_semName) {
    Check_Type(rbStr_semName, T_STRING);

    // Check length and format of semaphore name
    const char* sem_name = StringValueCStr(rbStr_semName);
    if (sem_name == NULL || strlen(sem_name) < 2 || sem_name[0] != '/') {
        rb_raise(rb_eRuntimeError, "invalid name. has to be in the format of '/foobar'");
    }
    if (strlen(sem_name) > 253) {
        rb_raise(rb_eRuntimeError, "name cannot be longer than 253 bytes long");
    }

    // Open the sem without O_CREAT flag so if we don't get a SEM_FAILED, it must exist
    if (sem_open(sem_name, 0) != SEM_FAILED) {
        return Qtrue;
    }
    else {
        int err = errno;
        switch (err) {
            case ENOENT:
                // Does not exist
                return Qfalse;
            case EACCES:
                // Exists but above our priv. level
                return Qtrue;
                break;
            case ENAMETOOLONG:
                rb_raise(rb_eRuntimeError, "name was too long (ENAMETOOLONG)");
                break;
            case EINVAL:
                rb_raise(rb_eRuntimeError, "name consists only of / and nothing else (EINVAL)");
                break;
            default:
                rb_raise(rb_eRuntimeError, "unknown errno: %d", err);
        }
    }
    return Qnil;
}

VALUE psem_initialize(VALUE self, VALUE rbStr_semName, VALUE rbNum_initialValue) {
    Check_Type(rbStr_semName, T_STRING);
    Check_Type(rbNum_initialValue, T_FIXNUM);

    // Check length and format of semaphore name
    const char* sem_name = StringValueCStr(rbStr_semName);
    if (sem_name == NULL || strlen(sem_name) < 2 || sem_name[0] != '/') {
        rb_raise(rb_eRuntimeError, "invalid name. has to be in the format of '/foobar'");
    }
    if (strlen(sem_name) > 251) {
        rb_raise(rb_eRuntimeError, "name cannot be longer than 251 bytes long");
    }

    // Check initial value is positive
    int initialValue = FIX2INT(rbNum_initialValue);
    if (initialValue < 0) {
        rb_raise(rb_eRuntimeError, "semaphores cannot have negative values");
    }

    // Create the semaphore
    sem_t* sem = sem_open(sem_name, O_CREAT, strtol("0644", 0, 8), initialValue);
    if (sem == SEM_FAILED) {
        rb_raise(rb_eRuntimeError, "sem_open() failed with errno: %d", errno);
    }

    // Set inner
    rb_ivar_set(self, rb_intern("inner"), (VALUE)sem);
    return self;
}

VALUE psem_post(VALUE self) {
    sem_t* sem = get_current_semaphore(self);
    if (sem_post(sem) == -1) {
        int err = errno;
        switch (err) {
            case EINVAL:
                rb_raise(rb_eRuntimeError, "inner semaphore is not a valid semaphore (EINVAL)");
                break;
            case EOVERFLOW:
                rb_raise(rb_eRuntimeError, "maximum allowable value for a semaphore would be exceeded (EOVERFLOW)");
                break;
            default:
                rb_raise(rb_eRuntimeError, "unknown errno: %d", err);
        }
    }
    return Qnil;
}

VALUE psem_wait(VALUE self) {
    sem_t* sem = get_current_semaphore(self);
    if (sem_wait(sem) == -1) {
        int err = errno;
        switch (err) {
            case EINTR:
                rb_raise(rb_eRuntimeError, "got interrupted by a signal handler (EINTR)");
                break;
            case EINVAL:
                rb_raise(rb_eRuntimeError, "inner semaphore is not a valid semaphore (EINVAL)");
                break;
            default:
                rb_raise(rb_eRuntimeError, "unknown errno: %d", err);
        }
    }
    return Qtrue;
}

VALUE psem_trywait(VALUE self) {
    sem_t* sem = get_current_semaphore(self);
    if (sem_trywait(sem) == -1) {
        int err = errno;
        switch (err) {
            case EAGAIN:
                return Qfalse;
            case EINTR:
                rb_raise(rb_eRuntimeError, "got interrupted by a signal handler (EINTR)");
                break;
            case EINVAL:
                rb_raise(rb_eRuntimeError, "inner semaphore is not a valid semaphore (EINVAL)");
                break;
            default:
                rb_raise(rb_eRuntimeError, "unknown errno: %d", err);
        }
    }
    return Qtrue;
}

VALUE psem_close(VALUE self) {
    sem_t* sem = get_current_semaphore(self);
    if (sem_close(sem) == -1) {
        int err = errno;
        switch (err) {
            case EINVAL:
                rb_raise(rb_eRuntimeError, "inner semaphore is not a valid semaphore (EINVAL)");
                break;
            default:
                rb_raise(rb_eRuntimeError, "unknown errno: %d", err);
        }
    }
    return Qnil;
}

VALUE psem_get_inner(VALUE self) {
    // Get inner as size_t type. Primarily for debugging purposes.
    size_t cast = (size_t)rb_ivar_get(self, rb_intern("inner"));
    return INT2NUM(cast);
}

void Init_psem() {
    VALUE psemClass = rb_define_class("PSem", rb_cObject);

    // Initialize (does not change the value of the semaphore if it already exists
    // If the initial value is important or if the semaphore's presence is important,
    // they can be accessed via psem_exists.
    rb_define_method(psemClass, "initialize", psem_initialize, 2);

    // View the inner sem_t pointer as a BigNum for debugging
    rb_define_method(psemClass, "get_inner", psem_get_inner, 0);

    // Get current value -- nonblocking but in Linux returns the number
    // of processes/threads waiting on the semaphore as a negative integer
    // if it has reached zero already.
    rb_define_method(psemClass, "get_value", psem_get_value, 0);

    // Increment owned
    rb_define_method(psemClass, "post", psem_post, 0);

    // Decrement owned
    rb_define_method(psemClass, "wait", psem_wait, 0);
    rb_define_method(psemClass, "try_wait", psem_trywait, 0);
    rb_define_method(psemClass, "wait_millis", psem_waitmillis, 1);

    // Unlink owned
    rb_define_method(psemClass, "close", psem_close, 0);

    // Unlink by name
    rb_define_singleton_method(psemClass, "unlink", psem_unlink, 1);

    // Check if exists
    rb_define_singleton_method(psemClass, "exists", psem_exists, 1);
}
