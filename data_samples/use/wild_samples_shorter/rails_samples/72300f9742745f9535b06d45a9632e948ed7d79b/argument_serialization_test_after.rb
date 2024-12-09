    assert_arguments_roundtrip [@person]
  end

  test "should keep Global IDs strings as they are" do
    assert_arguments_roundtrip [@person.to_gid.to_s]
  end

  test "should dive deep into arrays and hashes" do
    assert_arguments_roundtrip [3, [@person]]
    assert_arguments_roundtrip [{ "a" => @person }]
  end