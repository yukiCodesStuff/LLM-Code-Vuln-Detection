    assert_equal "only partial", @view.render("test/partial_only")
  end

  def test_render_outside_path
    assert File.exist?(File.join(File.dirname(__FILE__), '../../test/abstract_unit.rb'))
    assert_raises ActionView::MissingTemplate do
      @view.render(:template => "../\\../test/abstract_unit.rb")
    end
  end

  def test_render_partial
    assert_equal "only partial", @view.render(:partial => "test/partial_only")
  end
