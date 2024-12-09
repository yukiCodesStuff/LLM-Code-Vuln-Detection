  end

  def test_dynamic_render_with_file
    # This is extremely bad, but should be possible to do.
    assert File.exist?(File.expand_path("../../test/abstract_unit.rb", __dir__))
    response = assert_deprecated { get :dynamic_render_with_file, params: { id: '../\\../test/abstract_unit.rb' } }
    assert_equal File.read(File.expand_path("../../test/abstract_unit.rb", __dir__)),
      response.body
  end

  def test_dynamic_render_with_absolute_path
    file = Tempfile.new("name")

  def test_permitted_dynamic_render_file_hash
    assert File.exist?(File.expand_path("../../test/abstract_unit.rb", __dir__))
    response = assert_deprecated { get :dynamic_render_permit, params: { id: { file: '../\\../test/abstract_unit.rb' } } }
    assert_equal File.read(File.expand_path("../../test/abstract_unit.rb", __dir__)),
      response.body
  end

  def test_dynamic_render_file_hash
    assert_raises ArgumentError do