    assert_equal %(dont <\\/close> tags), j(%(dont </close> tags))
  end

  def test_escape_backtick
    assert_equal "\\`", escape_javascript("`")
  end

  def test_escape_dollar_sign
    assert_equal "\\$", escape_javascript("$")
  end

  def test_escape_javascript_with_safebuffer
    given = %('quoted' "double-quoted" new-line:\n </closed>)
    expect = %(\\'quoted\\' \\"double-quoted\\" new-line:\\n <\\/closed>)
    assert_equal expect, escape_javascript(given)