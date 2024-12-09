    end

    def get_session_id
      render plain: "#{request.session.id}"
    end

    def call_reset_session
      session[:bar]

  def test_prevents_session_fixation
    with_test_route_set do
      assert_nil @cache.read("_session_id:0xhax")

      cookies["_session_id"] = "0xhax"
      get "/set_session_value"

      assert_response :success
      assert_not_equal "0xhax", cookies["_session_id"]
      assert_nil @cache.read("_session_id:0xhax")
      assert_equal({ "foo" => "bar" }, @cache.read("_session_id:#{cookies['_session_id']}"))
    end
  end

  private