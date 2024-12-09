    )
  end

  test "logs error if parsing unsuccessful" do
    with_test_routing do
      output = StringIO.new
      json = "[\"person]\": {\"name\": \"David\"}}"