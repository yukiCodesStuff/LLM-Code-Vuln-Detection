  include routes.url_helpers

  include ActionView::Helpers::UrlHelper
  include ActionDispatch::Assertions::DomAssertions
  include ActionView::Context
  include RenderERBUtils


  def test_mail_to_with_javascript
    snippet = mail_to("me@domain.com", "My email", :encode => "javascript")
    assert_dom_equal "<script type=\"text/javascript\">eval(decodeURIComponent('%64%6f%63%75%6d%65%6e%74%2e%77%72%69%74%65%28%27%3c%61%20%68%72%65%66%3d%22%6d%61%69%6c%74%6f%3a%6d%65%40%64%6f%6d%61%69%6e%2e%63%6f%6d%22%3e%4d%79%20%65%6d%61%69%6c%3c%2f%61%3e%27%29%3b'))</script>", snippet
    assert snippet.html_safe?
  end

  def test_mail_to_with_javascript_unicode
    snippet = mail_to("unicode@example.com", "únicode", :encode => "javascript")
    assert_dom_equal "<script type=\"text/javascript\">eval(decodeURIComponent('%64%6f%63%75%6d%65%6e%74%2e%77%72%69%74%65%28%27%3c%61%20%68%72%65%66%3d%22%6d%61%69%6c%74%6f%3a%75%6e%69%63%6f%64%65%40%65%78%61%6d%70%6c%65%2e%63%6f%6d%22%3e%c3%ba%6e%69%63%6f%64%65%3c%2f%61%3e%27%29%3b'))</script>", snippet
    assert snippet.html_safe
  end

  def test_mail_with_options
    assert_dom_equal "<a href=\"&#109;&#97;&#105;&#108;&#116;&#111;&#58;%6d%65@%64%6f%6d%61%69%6e.%63%6f%6d\">&#109;&#101;&#40;&#97;&#116;&#41;&#100;&#111;&#109;&#97;&#105;&#110;&#46;&#99;&#111;&#109;</a>", mail_to("me@domain.com", nil, :encode => "hex", :replace_at => "(at)")
    assert_dom_equal "<a href=\"&#109;&#97;&#105;&#108;&#116;&#111;&#58;%6d%65@%64%6f%6d%61%69%6e.%63%6f%6d\">My email</a>", mail_to("me@domain.com", "My email", :encode => "hex", :replace_at => "(at)")
    assert_dom_equal "<a href=\"&#109;&#97;&#105;&#108;&#116;&#111;&#58;%6d%65@%64%6f%6d%61%69%6e.%63%6f%6d\">&#109;&#101;&#40;&#97;&#116;&#41;&#100;&#111;&#109;&#97;&#105;&#110;&#40;&#100;&#111;&#116;&#41;&#99;&#111;&#109;</a>", mail_to("me@domain.com", nil, :encode => "hex", :replace_at => "(at)", :replace_dot => "(dot)")
    assert_dom_equal "<script type=\"text/javascript\">eval(decodeURIComponent('%64%6f%63%75%6d%65%6e%74%2e%77%72%69%74%65%28%27%3c%61%20%68%72%65%66%3d%22%6d%61%69%6c%74%6f%3a%6d%65%40%64%6f%6d%61%69%6e%2e%63%6f%6d%22%3e%4d%79%20%65%6d%61%69%6c%3c%2f%61%3e%27%29%3b'))</script>", mail_to("me@domain.com", "My email", :encode => "javascript", :replace_at => "(at)", :replace_dot => "(dot)")
    assert_dom_equal "<script type=\"text/javascript\">eval(decodeURIComponent('%64%6f%63%75%6d%65%6e%74%2e%77%72%69%74%65%28%27%3c%61%20%68%72%65%66%3d%22%6d%61%69%6c%74%6f%3a%6d%65%40%64%6f%6d%61%69%6e%2e%63%6f%6d%22%3e%6d%65%28%61%74%29%64%6f%6d%61%69%6e%28%64%6f%74%29%63%6f%6d%3c%2f%61%3e%27%29%3b'))</script>", mail_to("me@domain.com", nil, :encode => "javascript", :replace_at => "(at)", :replace_dot => "(dot)")
  end

  # TODO: button_to looks at this ... why?
  def protect_against_forgery?