    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "127.0.0.1",
      "HOST" => "www.example.com",
      "action_dispatch.show_detailed_exceptions" => true
    }

    assert_response :forbidden
    assert_match "Blocked host: 127.0.0.1", response.body
  end

  test "blocks requests with spoofed relative X-FORWARDED-HOST" do
    @app = ActionDispatch::HostAuthorization.new(App, ["www.example.com"])

    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "//randomhost.com",
      "HOST" => "www.example.com",
      "action_dispatch.show_detailed_exceptions" => true
    }
    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "sub.domain.com",
      "HOST" => "domain.com",
      "action_dispatch.show_detailed_exceptions" => true
    }

    assert_response :forbidden
    assert_match "Blocked host: sub.domain.com", response.body
  end

  test "sub-sub domains should not be permitted" do
    @app = ActionDispatch::HostAuthorization.new(App, ".domain.com")

    get "/", env: {
      "HOST" => "secondary.sub.domain.com",
      "action_dispatch.show_detailed_exceptions" => true
    }