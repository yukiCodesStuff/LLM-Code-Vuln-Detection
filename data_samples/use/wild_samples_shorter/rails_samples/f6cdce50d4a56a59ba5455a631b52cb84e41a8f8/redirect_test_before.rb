    redirect_back_or_to "http://www.rubyonrails.org/", status: 307, allow_other_host: false
  end

  def unsafe_redirect
    redirect_to "http://www.rubyonrails.org/"
  end

    redirect_to "http:///www.rubyonrails.org/"
  end

  def only_path_redirect
    redirect_to action: "other_host", only_path: true
  end

    assert_equal "http://www.rubyonrails.org/", redirect_to_url
  end

  def test_redirect_back_with_explicit_fallback_kwarg
    referer = "http://www.example.com/coming/from"
    @request.env["HTTP_REFERER"] = referer

    end
  end

  def test_only_path_redirect
    with_raise_on_open_redirects do
      get :only_path_redirect
      assert_response :redirect