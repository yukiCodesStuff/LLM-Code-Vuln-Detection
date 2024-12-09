  end

  test "query string without equal" do
    assert_parses({ "action" => nil }, "action")
  end

  test "query string with empty key" do
    assert_parses(