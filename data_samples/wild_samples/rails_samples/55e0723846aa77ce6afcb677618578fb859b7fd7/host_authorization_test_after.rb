    get "/", env: {
      "HOST" => "attacker.com#x.example.com",
    }

    assert_response :forbidden
    assert_match "Blocked host: attacker.com#x.example.com", response.body
  end

  test "blocks requests to similar host" do
    @app = ActionDispatch::HostAuthorization.new(App, "sub.example.com")

    get "/", env: {
      "HOST" => "sub-example.com",
    }