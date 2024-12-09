    assert_equal "HTTP Token: Access denied.\n", @response.body, "Authentication header was not properly parsed"
  end

  test "successful authentication request with Bearer instead of Token" do
    @request.env["HTTP_AUTHORIZATION"] = "Bearer lifo"
    get :index
