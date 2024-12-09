
  def test_render_json_with_callback
    xhr :get, :render_json_hello_world_with_callback
    assert_equal 'alert({"hello":"world"})', @response.body
    assert_equal 'text/javascript', @response.content_type
  end

  def test_render_json_with_custom_content_type