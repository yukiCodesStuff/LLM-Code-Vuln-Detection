    assert_equal '<p class="song> play&gt;"></p>', tag.p(class: [raw("song>"), "play>"])
  end

  def test_skip_invalid_escaped_attributes
    ["&1;", "&#1dfa3;", "& #123;"].each do |escaped|
      assert_equal %(<a href="#{escaped.gsub(/&/, '&amp;')}" />), tag("a", href: escaped)
      assert_equal %(<a href="#{escaped.gsub(/&/, '&amp;')}"></a>), tag.a(href: escaped)