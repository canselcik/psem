Gem::Specification.new 'psem', '0.0.1' do |s|
  s.name        = 'psem'
  s.version     = '0.0.1'
  s.date        = '2019-02-22'
  s.summary     = 'A Simple Ruby Gem for POSIX Named Semaphores'
  s.description = <<-EOF
A named semaphore is identified by a name of the form /somename; that is, a null-terminated string of up to 251 characters consisting of an initial slash, followed by one or more characters, none of which are slashes. Two processes can operate on the same named semaphore by passing the same name to sem_open.

The sem_open function creates a new named semaphore or opens an existing named semaphore. After the semaphore has been opened, it can be operated on using sem_post and sem_wait. When a process has finished using the semaphore, it can use sem_close to close the semaphore. When all processes have finished using the semaphore, it can be removed from the system using sem_unlink.
  EOF
  s.authors     = ['Can Selcik']
  s.email       = 'selcik.can@gmail.com'
  s.files       = %w[lib/psem.rb ext/psem/psem.c ext/psem/extconf.rb test/psem_test.rb]
  s.extensions  = %w[ext/psem/extconf.rb]
  s.homepage    = 'http://rubygems.org/gems/psem'
  s.license     = 'MIT'
  s.metadata    = { 'source_code_uri' => 'https://github.com/canselcik/psem' }
end

