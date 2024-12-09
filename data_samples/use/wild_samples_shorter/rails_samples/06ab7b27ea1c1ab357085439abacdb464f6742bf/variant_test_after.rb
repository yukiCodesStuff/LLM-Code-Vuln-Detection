  test "service_url doesn't grow in length despite long variant options" do
    blob = create_file_blob(filename: "racecar.jpg")
    variant = blob.variant(font: "a" * 10_000).processed
    assert_operator variant.service_url.length, :<, 726
  end

  test "works for vips processor" do
    begin