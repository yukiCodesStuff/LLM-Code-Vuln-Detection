    assert_response :success
  end

  def test_chomps_slashes
    get :index, params: { form_path: "/per_form_tokens/post_one?foo=bar" }

    form_token = assert_presence_and_fetch_form_csrf_token