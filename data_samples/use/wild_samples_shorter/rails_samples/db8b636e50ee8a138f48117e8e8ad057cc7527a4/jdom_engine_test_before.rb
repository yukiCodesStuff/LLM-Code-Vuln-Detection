  require 'active_support/xml_mini'
  require 'active_support/core_ext/hash/conversions'

  class JDOMEngineTest < ActiveSupport::TestCase
    include ActiveSupport

    def setup
      @default_backend = XmlMini.backend
      XmlMini.backend = 'JDOM'
    end
       assert_equal 'image/png', file.content_type
    end

    def test_exception_thrown_on_expansion_attack
      assert_raise NativeException do
        attack_xml = <<-EOT
      <?xml version="1.0" encoding="UTF-8"?>
      <!DOCTYPE member [
        <!ENTITY a "&b;&b;&b;&b;&b;&b;&b;&b;&b;&b;">
        <!ENTITY b "&c;&c;&c;&c;&c;&c;&c;&c;&c;&c;">
        <!ENTITY c "&d;&d;&d;&d;&d;&d;&d;&d;&d;&d;">