    assert_equal "<ok>bar</ok>", resp.body
  end

  test "parses hash params" do
    with_test_routing do
      xml = "<person><name>David</name></person>"
      post "/parse", xml, default_headers