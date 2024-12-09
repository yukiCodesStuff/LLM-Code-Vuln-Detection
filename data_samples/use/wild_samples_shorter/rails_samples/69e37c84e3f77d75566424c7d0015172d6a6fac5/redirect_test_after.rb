    redirect_to "///www.rubyonrails.org/"
  end

  def unsafe_redirect_with_illegal_http_header_value_character
    redirect_to "javascript:alert(document.domain)\b", allow_other_host: true
  end

  def only_path_redirect
    redirect_to action: "other_host", only_path: true
  end

    end
  end

  def test_unsafe_redirect_with_illegal_http_header_value_character
    with_raise_on_open_redirects do
      error = assert_raise(ActionController::Redirecting::UnsafeRedirectError) do
        get :unsafe_redirect_with_illegal_http_header_value_character
      end

      msg = "The redirect URL javascript:alert(document.domain)\b contains one or more illegal HTTP header field character. " \
        "Set of legal characters defined in https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6"

      assert_equal msg, error.message
    end
  end


  def test_only_path_redirect
    with_raise_on_open_redirects do
      get :only_path_redirect
      assert_response :redirect