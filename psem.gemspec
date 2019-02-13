Gem::Specification.new 'psem', '0.2.1' do |s|
  s.name        = 'psem'
  s.version     = '0.2.1'
  s.date        = '2019-02-13'
  s.summary     = 'A Simple Ruby Gem for POSIX Named Semaphores'
  s.description = 'Supports named semaphores both on Linux and OSX. "get_value" and "timedwait" is deprecated on OSX but everything else is functional and safe.'
  s.authors     = ['Can Selcik']
  s.email       = 'selcik.can@gmail.com'
  s.files       = %w[lib/psem.rb ext/psem/psem.c ext/psem/extconf.rb test/psem_test.rb]
  s.extensions  = %w[ext/psem/extconf.rb]
  s.homepage    = 'http://rubygems.org/gems/psem'
  s.license     = 'MIT'
  s.metadata    = { 'source_code_uri' => 'https://github.com/canselcik/psem' }
end

