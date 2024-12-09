    assert_match "Blocked host: attacker.com#x.example.com", response.body
  end

  test "config setting action_dispatch.hosts_response_app is deprecated" do
    assert_deprecated do
      ActionDispatch::HostAuthorization.new(App, "example.com", ->(env) { true })
    end