    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "127.0.0.1",
      "HOST" => "www.example.com",
      "action_dispatch.show_detailed_exceptions" => true
    }

    assert_response :forbidden
    assert_match "Blocked host: 127.0.0.1", response.body
  end

  test "does not consider IP addresses in X-FORWARDED-HOST spoofed when disabled" do
    @app = ActionDispatch::HostAuthorization.new(App, nil)

    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "127.0.0.1",
      "HOST" => "www.example.com",
    }
    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "sub.domain.com",
      "HOST" => "domain.com",
      "action_dispatch.show_detailed_exceptions" => true
    }

    assert_response :forbidden
    assert_match "Blocked host: sub.domain.com", response.body
  end

  test "forwarded hosts are allowed when permitted" do
    @app = ActionDispatch::HostAuthorization.new(App, ".domain.com")

    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "sub.domain.com",
      "HOST" => "domain.com",
    }