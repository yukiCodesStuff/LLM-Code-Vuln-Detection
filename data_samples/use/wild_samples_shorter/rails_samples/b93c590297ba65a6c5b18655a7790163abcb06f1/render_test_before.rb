    render :action => "hello_world"
  end

  def render_action_hello_world_as_string
    render "hello_world"
  end

    assert_template "test/hello_world"
  end

  # :ported:
  def test_render_action_hello_world_as_string
    get :render_action_hello_world_as_string
    assert_equal "Hello world!", @response.body