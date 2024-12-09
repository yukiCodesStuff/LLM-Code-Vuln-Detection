    assert_equal "gyroscope 'self'", response.headers[ActionDispatch::Constants::FEATURE_POLICY]
  end

  test "non-html requests will set a policy" do
    @app = build_app(->(env) { [200, { Rack::CONTENT_TYPE => "application/json" }, []] })

    get "/index"

    assert_equal "gyroscope 'self'", response.headers[ActionDispatch::Constants::FEATURE_POLICY]
  end

  test "existing policies will not be overwritten" do
    @app = build_app(->(env) { [200, { ActionDispatch::Constants::FEATURE_POLICY => "gyroscope 'none'" }, []] })