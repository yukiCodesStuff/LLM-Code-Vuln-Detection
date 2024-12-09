   %(<IMG SRC="jav&#x0A;ascript:alert('XSS');">),
   %(<IMG SRC="jav&#x0D;ascript:alert('XSS');">),
   %(<IMG SRC=" &#14;  javascript:alert('XSS');">),
   %(<IMG SRC="javascript&#x3a;alert('XSS');">),
   %(<IMG SRC=`javascript:alert("RSnake says, 'XSS'")`>)].each_with_index do |img_hack, i|
    define_method "test_should_not_fall_for_xss_image_hack_#{i+1}" do
      assert_sanitized img_hack, "<img>"
    end
    assert_equal '', sanitize_css(raw)
  end

  def test_should_sanitize_across_newlines
    raw = %(\nwidth:\nexpression(alert('XSS'));\n)
    assert_equal '', sanitize_css(raw)
  end

  def test_should_sanitize_img_vbscript
    assert_sanitized %(<img src='vbscript:msgbox("XSS")' />), '<img />'
  end

    assert_sanitized "<span class=\"\\", "<span class=\"\\\">"
  end

  def test_x03a
    assert_sanitized %(<a href="javascript&#x3a;alert('XSS');">), "<a>"
    assert_sanitized %(<a href="javascript&#x003a;alert('XSS');">), "<a>"
    assert_sanitized %(<a href="http&#x3a;//legit">), %(<a href="http://legit">)
    assert_sanitized %(<a href="javascript&#x3A;alert('XSS');">), "<a>"
    assert_sanitized %(<a href="javascript&#x003A;alert('XSS');">), "<a>"
    assert_sanitized %(<a href="http&#x3A;//legit">), %(<a href="http://legit">)
  end

protected
  def assert_sanitized(input, expected = nil)
    @sanitizer ||= HTML::WhiteListSanitizer.new
    if input