    assert_equal "<ok>bar</ok>", resp.body
  end

  def assert_parses(expected, xml)
    with_test_routing do
      post "/parse", xml, default_headers
      assert_response :ok
      assert_equal(expected, TestController.last_request_parameters)
    end
  end

  test "nils are stripped from collections" do
    assert_parses(
      {"hash" => { "person" => nil} },
      "<hash><person type=\"array\"><person nil=\"true\"/></person></hash>")
    assert_parses(
      {"hash" => { "person" => ['foo']} },
      "<hash><person type=\"array\"><person>foo</person><person nil=\"true\"/></person>\n</hash>")
  end

  test "parses hash params" do
    with_test_routing do
      xml = "<person><name>David</name></person>"
      post "/parse", xml, default_headers