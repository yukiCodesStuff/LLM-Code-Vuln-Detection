  end

  test "query string without equal" do
    assert_parses({"action" => nil}, "action")
    assert_parses({"action" => {"foo" => nil}}, "action[foo]")
    assert_parses({"action" => {"foo" => { "bar" => nil }}}, "action[foo][bar]")
    assert_parses({"action" => {"foo" => { "bar" => nil }}}, "action[foo][bar][]")
    assert_parses({"action" => {"foo" => nil}}, "action[foo][]")
    assert_parses({"action"=>{"foo"=>[{"bar"=>nil}]}}, "action[foo][][bar]")
  end

  test "query string with empty key" do
    assert_parses(