    [nil, '', '   '].each do |blank|
      stripped = strip_tags(blank)
      assert_equal blank, stripped
    end
    assert_equal "", strip_tags("<script>")
    assert_equal "something &lt;img onerror=alert(1337)", ERB::Util.html_escape(strip_tags("something <img onerror=alert(1337)"))
  end

  def test_sanitize_is_marked_safe
    assert sanitize("<html><script></script></html>").html_safe?