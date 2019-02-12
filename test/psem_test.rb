require 'minitest/autorun'
require 'psem/psem'

class PSemTest < MiniTest::Unit::TestCase
  def test_sanity
    assert_equal 'ok', PSem.new.working
  end
end
