    )
  end

  test "nils are stripped from collections" do
    assert_parses(
      {"person" => nil},
      "{\"person\":[null]}", { 'CONTENT_TYPE' => 'application/json' }
    )
    assert_parses(
      {"person" => ['foo']},
      "{\"person\":[\"foo\",null]}", { 'CONTENT_TYPE' => 'application/json' }
    )
    assert_parses(
      {"person" => nil},
      "{\"person\":[null, null]}", { 'CONTENT_TYPE' => 'application/json' }
    )
  end

  test "logs error if parsing unsuccessful" do
    with_test_routing do
      output = StringIO.new
      json = "[\"person]\": {\"name\": \"David\"}}"