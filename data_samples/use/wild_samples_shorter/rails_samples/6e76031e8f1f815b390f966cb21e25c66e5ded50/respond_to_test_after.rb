  def test_json_with_callback_sets_javascript_content_type
    @request.accept = 'application/json'
    get :json_with_callback
    assert_equal '/**/alert(JS)', @response.body
    assert_equal 'text/javascript', @response.content_type
  end

  def test_xhr