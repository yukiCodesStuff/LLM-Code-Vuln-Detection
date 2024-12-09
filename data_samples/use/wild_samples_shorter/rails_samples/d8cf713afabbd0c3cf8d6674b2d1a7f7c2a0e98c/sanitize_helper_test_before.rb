    [nil, '', '   '].each do |blank|
      stripped = strip_tags(blank)
      assert_equal blank, stripped
      assert stripped.html_safe? unless blank.nil?
    end
    assert strip_tags("<script>").html_safe?
  end

  def test_sanitize_is_marked_safe
    assert sanitize("<html><script></script></html>").html_safe?