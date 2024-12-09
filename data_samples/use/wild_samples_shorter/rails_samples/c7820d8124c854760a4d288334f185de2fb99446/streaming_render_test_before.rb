  end

  def test_render_file
    assert_equal "Hello world!", buffered_render(file: "test/hello_world")
  end

  def test_render_file_with_locals
    locals = { secret: "in the sauce" }
    assert_equal "The secret is in the sauce\n", buffered_render(file: "test/render_file_with_locals", locals: locals)
  end

  def test_render_partial
    assert_equal "only partial", buffered_render(partial: "test/partial_only")