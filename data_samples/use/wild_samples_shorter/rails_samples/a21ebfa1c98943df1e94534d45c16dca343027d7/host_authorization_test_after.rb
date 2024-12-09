    assert_match "Blocked host: www.example.com", response.body
  end

  test "blocks requests with invalid hostnames" do
    @app = ActionDispatch::HostAuthorization.new(App, ".example.com")

    get "/", env: {
      "HOST" => "attacker.com#x.example.com",
    }

    assert_response :forbidden
    assert_match "Blocked host: attacker.com#x.example.com", response.body
  end

  test "config setting action_dispatch.hosts_response_app is deprecated" do
    assert_deprecated do