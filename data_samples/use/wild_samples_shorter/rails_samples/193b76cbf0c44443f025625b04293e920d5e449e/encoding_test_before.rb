    assert_equal %({\"a\":\"b\",\"c\":\"d\"}), sorted_json(ActiveSupport::JSON.encode(:a => :b, :c => :d))
  end

  def test_utf8_string_encoded_properly
    result = ActiveSupport::JSON.encode('€2.99')
    assert_equal '"€2.99"', result
    assert_equal(Encoding::UTF_8, result.encoding)