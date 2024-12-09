    assert_equal '<p class="song> play&gt;"></p>', tag.p(class: [raw("song>"), "play>"])
  end

  def test_tag_does_not_honor_html_safe_double_quotes_as_attributes
    assert_dom_equal '<p title="&quot;">content</p>',
      content_tag('p', "content", title: '"'.html_safe)
  end

  def test_data_tag_does_not_honor_html_safe_double_quotes_as_attributes
    assert_dom_equal '<p data-title="&quot;">content</p>',
      content_tag('p', "content", data: { title: '"'.html_safe })
  end

  def test_skip_invalid_escaped_attributes
    ["&1;", "&#1dfa3;", "& #123;"].each do |escaped|
      assert_equal %(<a href="#{escaped.gsub(/&/, '&amp;')}" />), tag("a", href: escaped)
      assert_equal %(<a href="#{escaped.gsub(/&/, '&amp;')}"></a>), tag.a(href: escaped)