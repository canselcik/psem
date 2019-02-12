require 'mkmf'

have_header 'pthread.h'
have_header 'semaphore.h'

create_makefile 'psem/psem'
