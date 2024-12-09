    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "sub.domain.com",
      "HOST" => "domain.com",
    }

    assert_response :ok
    assert_equal "Success", body
  end

  test "exclude matches allow any host" do
    @app = ActionDispatch::HostAuthorization.new(App, "only.com", exclude: ->(req) { req.path == "/foo" })

    get "/foo"

    assert_response :ok
    assert_equal "Success", body
  end

  test "exclude misses block unallowed hosts" do
    @app = ActionDispatch::HostAuthorization.new(App, "only.com", exclude: ->(req) { req.path == "/bar" })

    get "/foo"

    assert_response :forbidden
    assert_match "Blocked host: www.example.com", response.body
  end

  test "only compare to valid hostnames" do
    @app = ActionDispatch::HostAuthorization.new(App, ".example.com")

    get "/", env: {
      "HOST" => "example.com#sub.example.com",
    }