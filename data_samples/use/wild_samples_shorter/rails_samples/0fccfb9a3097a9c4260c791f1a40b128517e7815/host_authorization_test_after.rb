    assert_match "Blocked host: 127.0.0.1", response.body
  end

  test "blocks requests with spoofed relative X-FORWARDED-HOST" do
    @app = ActionDispatch::HostAuthorization.new(App, ["www.example.com"])

    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "//randomhost.com",
      "HOST" => "www.example.com",
      "action_dispatch.show_detailed_exceptions" => true
    }

    assert_response :forbidden
    assert_match "Blocked host: //randomhost.com", response.body
  end

  test "forwarded secondary hosts are allowed when permitted" do
    @app = ActionDispatch::HostAuthorization.new(App, ".domain.com")

    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "example.com, my-sub.domain.com",
      "HOST" => "domain.com",
    }

    assert_response :ok
    assert_equal "Success", body
  end

  test "forwarded secondary hosts are blocked when mismatch" do
    @app = ActionDispatch::HostAuthorization.new(App, "domain.com")

    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "domain.com, evil.com",
      "HOST" => "domain.com",
      "action_dispatch.show_detailed_exceptions" => true
    }

    assert_response :forbidden
    assert_match "Blocked host: evil.com", response.body
  end

  test "does not consider IP addresses in X-FORWARDED-HOST spoofed when disabled" do
    @app = ActionDispatch::HostAuthorization.new(App, nil)

    get "/", env: {
    assert_match "Blocked host: sub.domain.com", response.body
  end

  test "sub-sub domains should not be permitted" do
    @app = ActionDispatch::HostAuthorization.new(App, ".domain.com")

    get "/", env: {
      "HOST" => "secondary.sub.domain.com",
      "action_dispatch.show_detailed_exceptions" => true
    }

    assert_response :forbidden
    assert_match "Blocked host: secondary.sub.domain.com", response.body
  end

  test "forwarded hosts are allowed when permitted" do
    @app = ActionDispatch::HostAuthorization.new(App, ".domain.com")

    get "/", env: {
      "HTTP_X_FORWARDED_HOST" => "my-sub.domain.com",
      "HOST" => "domain.com",
    }

    assert_response :ok
    assert_equal "Success", body
  end

  test "lots of NG hosts" do
    ng_hosts = [
      "hacker%E3%80%82com",
      "hacker%00.com",
      "www.theirsite.com@yoursite.com",
      "hacker.com/test/",
      "hacker%252ecom",
      ".hacker.com",
      "/\/\/hacker.com/",
      "/hacker.com",
      "../hacker.com",
      ".hacker.com",
      "@hacker.com",
      "hacker.com",
      "hacker.com%23@example.com",
      "hacker.com/.jpg",
      "hacker.com\texample.com/",
      "hacker.com/example.com",
      "hacker.com\@example.com",
      "hacker.com/example.com",
      "hacker.com/"
    ]

    @app = ActionDispatch::HostAuthorization.new(App, "example.com")

    ng_hosts.each do |host|
      get "/", env: {
        "HTTP_X_FORWARDED_HOST" => host,
        "HOST" => "example.com",
        "action_dispatch.show_detailed_exceptions" => true
      }

      assert_response :forbidden
      assert_match "Blocked host: #{host}", response.body
    end
  end

  test "exclude matches allow any host" do
    @app = ActionDispatch::HostAuthorization.new(App, "only.com", exclude: ->(req) { req.path == "/foo" })

    get "/foo"