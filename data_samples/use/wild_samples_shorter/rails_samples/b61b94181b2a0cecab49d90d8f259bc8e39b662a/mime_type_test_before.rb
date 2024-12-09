    assert_raises Mime::Type::InvalidMimeType do
      Mime::Type.new(nil)
    end
  end

  test "holds a reference to mime symbols" do
    old_symbols = Mime::SET.symbols