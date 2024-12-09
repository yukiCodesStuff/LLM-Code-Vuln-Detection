    assert_deprecated do
      @lookup_context.with_fallbacks do
        assert_equal 3, @lookup_context.view_paths.size
        assert_includes @lookup_context.view_paths, ActionView::FallbackFileSystemResolver.new("")
        assert_includes @lookup_context.view_paths, ActionView::FallbackFileSystemResolver.new("/")
      end
    end

    @lookup_context = @lookup_context.with_fallbacks

    assert_equal 3, @lookup_context.view_paths.size
    assert_includes @lookup_context.view_paths, ActionView::FallbackFileSystemResolver.new("")
    assert_includes @lookup_context.view_paths, ActionView::FallbackFileSystemResolver.new("/")
  end

  test "add fallbacks just once in nested fallbacks calls" do
    assert_deprecated do