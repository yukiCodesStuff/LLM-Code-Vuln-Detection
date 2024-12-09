  require 'active_support/xml_mini'
  require 'active_support/core_ext/hash/conversions'


  class JDOMEngineTest < ActiveSupport::TestCase
    include ActiveSupport

    FILES_DIR = File.dirname(__FILE__) + '/../fixtures/xml'

    def setup
      @default_backend = XmlMini.backend
      XmlMini.backend = 'JDOM'
    end
       assert_equal 'image/png', file.content_type
    end

    def test_not_allowed_to_expand_entities_to_files
      attack_xml = <<-EOT
      <!DOCTYPE member [
        <!ENTITY a SYSTEM "file://#{FILES_DIR}/jdom_include.txt">
      ]>
      <member>x&a;</member>
      EOT
      assert_equal 'x', Hash.from_xml(attack_xml)["member"]
    end

  def test_not_allowed_to_expand_parameter_entities_to_files
      attack_xml = <<-EOT
      <!DOCTYPE member [
        <!ENTITY % b SYSTEM "file://#{FILES_DIR}/jdom_entities.txt">
        %b;
      ]>
      <member>x&a;</member>
      EOT
      assert_raise Java::OrgXmlSax::SAXParseException do
        assert_equal 'x', Hash.from_xml(attack_xml)["member"]
      end
    end


    def test_not_allowed_to_load_external_doctypes
      attack_xml = <<-EOT
      <!DOCTYPE member SYSTEM "file://#{FILES_DIR}/jdom_doctype.dtd">
      <member>x&a;</member>
      EOT
      assert_equal 'x', Hash.from_xml(attack_xml)["member"]
    end

    def test_exception_thrown_on_expansion_attack
      assert_raise Java::OrgXmlSax::SAXParseException do
        attack_xml = <<-EOT
      <!DOCTYPE member [
        <!ENTITY a "&b;&b;&b;&b;&b;&b;&b;&b;&b;&b;">
        <!ENTITY b "&c;&c;&c;&c;&c;&c;&c;&c;&c;&c;">
        <!ENTITY c "&d;&d;&d;&d;&d;&d;&d;&d;&d;&d;">