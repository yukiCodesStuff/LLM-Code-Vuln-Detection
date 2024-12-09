        <replies-close-in type="integer">2592000000</replies-close-in>
        <written-on type="date">2003-07-16</written-on>
        <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
        <author-email-address>david@loudthinking.com</author-email-address>
        <parent-id></parent-id>
        <ad-revenue type="decimal">1.5</ad-revenue>
        <optimum-viewing-angle type="float">135</optimum-viewing-angle>
      </topic>
    EOT

    expected_topic_hash = {
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil,
      :ad_revenue => BigDecimal("1.50"),
      :optimum_viewing_angle => 135.0,
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end
        <approved type="boolean"></approved>
        <written-on type="date"></written-on>
        <viewed-at type="datetime"></viewed-at>
        <parent-id></parent-id>
      </topic>
    EOT

      :approved   => nil,
      :written_on => nil,
      :viewed_at  => nil,
      :parent_id  => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
    assert_equal expected_product_hash, Hash.from_xml(product_xml)["product"]
  end

  def test_from_xml_raises_on_disallowed_type_attributes
    assert_raise ActiveSupport::XMLConverter::DisallowedType do
      Hash.from_xml '<product><name type="foo">value</name></product>', %w(foo)
    end
  end

  def test_from_xml_disallows_symbol_and_yaml_types_by_default
    assert_raise ActiveSupport::XMLConverter::DisallowedType do
      Hash.from_xml '<product><name type="symbol">value</name></product>'
    end

    assert_raise ActiveSupport::XMLConverter::DisallowedType do
      Hash.from_xml '<product><name type="yaml">value</name></product>'
    end
  end

  def test_from_trusted_xml_allows_symbol_and_yaml_types
    expected = { 'product' => { 'name' => :value }}
    assert_equal expected, Hash.from_trusted_xml('<product><name type="symbol">value</name></product>')
    assert_equal expected, Hash.from_trusted_xml('<product><name type="yaml">:value</name></product>')
  end

  def test_should_use_default_value_for_unknown_key
    hash_wia = HashWithIndifferentAccess.new(3)
    assert_equal 3, hash_wia[:new_key]
  end