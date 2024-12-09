    end

    def get_session_id
      render plain: "#{request.session.id.public_id}"
    end

    def call_reset_session
      session[:bar]

  def test_prevents_session_fixation
    with_test_route_set do
      sid = Rack::Session::SessionId.new("0xhax")
      assert_nil @cache.read("_session_id:#{sid.private_id}")

      cookies["_session_id"] = sid.public_id
      get "/set_session_value"

      assert_response :success
      assert_not_equal sid.public_id, cookies["_session_id"]
      assert_nil @cache.read("_session_id:#{sid.private_id}")
      assert_equal(
        { "foo" => "bar" },
        @cache.read("_session_id:#{Rack::Session::SessionId.new(cookies['_session_id']).private_id}")
      )
    end
  end

  def test_can_read_session_with_legacy_id
    with_test_route_set do
      get "/set_session_value"
      assert_response :success
      assert cookies["_session_id"]

      sid = Rack::Session::SessionId.new(cookies['_session_id'])
      session = @cache.read("_session_id:#{sid.private_id}")
      @cache.delete("_session_id:#{sid.private_id}")
      @cache.write("_session_id:#{sid.public_id}", session)

      get "/get_session_value"
      assert_response :success
      assert_equal 'foo: "bar"', response.body
    end
  end

  def test_drop_session_in_the_legacy_id_as_well
    with_test_route_set do
      get "/set_session_value"
      assert_response :success
      assert cookies["_session_id"]

      sid = Rack::Session::SessionId.new(cookies['_session_id'])
      session = @cache.read("_session_id:#{sid.private_id}")
      @cache.delete("_session_id:#{sid.private_id}")
      @cache.write("_session_id:#{sid.public_id}", session)

      get "/call_reset_session"
      assert_response :success
      assert_not_equal [], headers["Set-Cookie"]

      assert_nil @cache.read("_session_id:#{sid.private_id}")
      assert_nil @cache.read("_session_id:#{sid.public_id}")
    end
  end

  private