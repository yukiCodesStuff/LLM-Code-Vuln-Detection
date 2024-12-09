    assert_equal "HTTP Token: Access denied.\n", @response.body, "Authentication header was not properly parsed"
  end

  test "authentication request with evil header" do
    @request.env["HTTP_AUTHORIZATION"] = "Token ." + " " * (1024*80-8) + "."
    Timeout.timeout(1) do
      get :index
    end

    assert_response :unauthorized
    assert_equal "HTTP Token: Access denied.\n", @response.body, "Authentication header was not properly parsed"
  end

  test "successful authentication request with Bearer instead of Token" do
    @request.env["HTTP_AUTHORIZATION"] = "Bearer lifo"
    get :index
