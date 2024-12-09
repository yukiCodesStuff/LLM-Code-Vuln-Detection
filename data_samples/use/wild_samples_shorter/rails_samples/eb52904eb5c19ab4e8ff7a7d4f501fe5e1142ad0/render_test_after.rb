  end

  def test_dynamic_render_with_file
    assert File.exist?(File.expand_path("../../test/abstract_unit.rb", __dir__))
    assert_deprecated do
      assert_raises ActionView::MissingTemplate do
        get :dynamic_render_with_file, params: { id: '../\\../test/abstract_unit.rb' }
      end
    end
  end

  def test_dynamic_render_with_absolute_path
    file = Tempfile.new("name")

  def test_permitted_dynamic_render_file_hash
    assert File.exist?(File.expand_path("../../test/abstract_unit.rb", __dir__))
    assert_deprecated do
      assert_raises ActionView::MissingTemplate do
        get :dynamic_render_permit, params: { id: { file: '../\\../test/abstract_unit.rb' } }
      end
    end
  end

  def test_dynamic_render_file_hash
    assert_raises ArgumentError do