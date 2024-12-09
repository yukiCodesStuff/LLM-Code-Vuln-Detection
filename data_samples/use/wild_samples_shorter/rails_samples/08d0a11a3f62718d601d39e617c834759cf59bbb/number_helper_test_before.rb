    assert_equal "555-1234", number_to_phone(5551234)
    assert_equal "(800) 555-1212 x 123", number_to_phone(8005551212, area_code: true, extension: 123)
    assert_equal "+18005551212", number_to_phone(8005551212, country_code: 1, delimiter: "")
  end

  def test_number_to_currency
    assert_equal nil, number_to_currency(nil)
    assert_equal "$1,234,567,892", number_to_currency(1234567891.50, precision: 0)
    assert_equal "1,234,567,890.50 - K&#269;", number_to_currency("-1234567890.50", unit: raw("K&#269;"), format: "%n %u", negative_format: "%n - %u")
    assert_equal "&amp;pound;1,234,567,890.50", number_to_currency("1234567890.50", unit: "&pound;")
  end

  def test_number_to_percentage
    assert_equal nil, number_to_percentage(nil)
    assert_equal "100.000%", number_to_percentage(100)
    assert_equal "100%", number_to_percentage(100, precision: 0)
    assert_equal "123.4%", number_to_percentage(123.400, precision: 3, strip_insignificant_zeros: true)
    assert_equal "1.000,000%", number_to_percentage(1000, delimiter: ".", separator: ",")
  end
    assert_equal "489.0 Thousand", number_to_human(489000, precision: 4, strip_insignificant_zeros: false)
  end

  def test_number_helpers_escape_delimiter_and_separator
    assert_equal "111&lt;script&gt;&lt;/script&gt;111&lt;script&gt;&lt;/script&gt;1111", number_to_phone(1111111111, delimiter: "<script></script>")

    assert_equal "$1&lt;script&gt;&lt;/script&gt;01", number_to_currency(1.01, separator: "<script></script>")
    assert_equal "100&lt;script&gt;&lt;/script&gt;000 Quadrillion", number_to_human(10**20, delimiter: "<script></script>")
  end

  def test_number_helpers_outputs_are_html_safe
    assert number_to_human(1).html_safe?
    assert !number_to_human("<script></script>").html_safe?
    assert number_to_human("asdf".html_safe).html_safe?