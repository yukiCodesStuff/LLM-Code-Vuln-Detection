    assert_response :success
  end

  def test_does_not_return_old_csrf_token
    get :index

    token = @controller.send(:form_authenticity_token)

    unmasked_token = @controller.send(:unmask_token, Base64.urlsafe_decode64(token))

    assert_not_equal @controller.send(:real_csrf_token, session), unmasked_token
  end

  def test_returns_hmacd_token
    get :index

    token = @controller.send(:form_authenticity_token)

    unmasked_token = @controller.send(:unmask_token, Base64.urlsafe_decode64(token))

    assert_equal @controller.send(:global_csrf_token, session), unmasked_token
  end

  def test_accepts_old_csrf_token
    get :index

    non_hmac_token = @controller.send(:mask_token, @controller.send(:real_csrf_token, session))

    # This is required because PATH_INFO isn't reset between requests.
    @request.env["PATH_INFO"] = "/per_form_tokens/post_one"
    assert_nothing_raised do
      post :post_one, params: { custom_authenticity_token: non_hmac_token }
    end
    assert_response :success
  end

  def test_chomps_slashes
    get :index, params: { form_path: "/per_form_tokens/post_one?foo=bar" }

    form_token = assert_presence_and_fetch_form_csrf_token