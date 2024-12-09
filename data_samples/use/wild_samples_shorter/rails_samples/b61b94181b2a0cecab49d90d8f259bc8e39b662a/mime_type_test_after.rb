    assert_raises Mime::Type::InvalidMimeType do
      Mime::Type.new(nil)
    end

    assert_raises Mime::Type::InvalidMimeType do
      Timeout.timeout(1) do # Shouldn't take more than 1s
        Mime::Type.new("text/html ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0 ;0;")
      end
    end
  end

  test "holds a reference to mime symbols" do
    old_symbols = Mime::SET.symbols