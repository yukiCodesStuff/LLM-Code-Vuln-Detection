    assert_equal "only partial", @view.render("test/partial_only")
  end

  def test_render_partial
    assert_equal "only partial", @view.render(:partial => "test/partial_only")
  end
