    assert_match "Blocked host: www.example.com", response.body
  end

  test "only compare to valid hostnames" do
    @app = ActionDispatch::HostAuthorization.new(App, ".example.com")

    get "/", env: {
      "HOST" => "example.com#sub.example.com",
    }

    assert_response :forbidden
    assert_match "Blocked host: example.com#sub.example.com", response.body
  end

  test "config setting action_dispatch.hosts_response_app is deprecated" do
    assert_deprecated do
      ActionDispatch::HostAuthorization.new(App, "example.com", ->(env) { true })
    end