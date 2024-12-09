    assert_equal nil, number_to_currency(nil)
    assert_equal "$1,234,567,890.50", number_to_currency(1234567890.50)
    assert_equal "$1,234,567,892", number_to_currency(1234567891.50, precision: 0)
    assert_equal "1,234,567,890.50 - K&#269;", number_to_currency("-1234567890.50", unit: "K&#269;", format: "%n %u", negative_format: "%n - %u")
  end

  def test_number_to_percentage
    assert_equal nil, number_to_percentage(nil)