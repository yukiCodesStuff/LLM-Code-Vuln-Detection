    redirect_back_or_to "http://www.rubyonrails.org/", status: 307, allow_other_host: false
  end

  def safe_redirect_to_root
    redirect_to url_from("/")
  end

  def unsafe_redirect
    redirect_to "http://www.rubyonrails.org/"
  end

    redirect_to "http:///www.rubyonrails.org/"
  end

  def unsafe_redirect_protocol_relative_double_slash
    redirect_to "//www.rubyonrails.org/"
  end

  def unsafe_redirect_protocol_relative_triple_slash
    redirect_to "///www.rubyonrails.org/"
  end

  def only_path_redirect
    redirect_to action: "other_host", only_path: true
  end

    assert_equal "http://www.rubyonrails.org/", redirect_to_url
  end

  def test_safe_redirect_to_root
    get :safe_redirect_to_root

    assert_equal "http://test.host/", redirect_to_url
  end

  def test_redirect_back_with_explicit_fallback_kwarg
    referer = "http://www.example.com/coming/from"
    @request.env["HTTP_REFERER"] = referer

    end
  end

  def test_unsafe_redirect_with_protocol_relative_double_slash_url
    with_raise_on_open_redirects do
      error = assert_raise(ActionController::Redirecting::UnsafeRedirectError) do
        get :unsafe_redirect_protocol_relative_double_slash
      end

      assert_equal "Unsafe redirect to \"//www.rubyonrails.org/\", pass allow_other_host: true to redirect anyway.", error.message
    end
  end

  def test_unsafe_redirect_with_protocol_relative_triple_slash_url
    with_raise_on_open_redirects do
      error = assert_raise(ActionController::Redirecting::UnsafeRedirectError) do
        get :unsafe_redirect_protocol_relative_triple_slash
      end

      assert_equal "Unsafe redirect to \"///www.rubyonrails.org/\", pass allow_other_host: true to redirect anyway.", error.message
    end
  end

  def test_only_path_redirect
    with_raise_on_open_redirects do
      get :only_path_redirect
      assert_response :redirect