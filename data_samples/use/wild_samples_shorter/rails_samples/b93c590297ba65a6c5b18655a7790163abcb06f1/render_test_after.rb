    render :action => "hello_world"
  end

  def render_action_upcased_hello_world
    render :action => "Hello_world"
  end

  def render_action_hello_world_as_string
    render "hello_world"
  end

    assert_template "test/hello_world"
  end

  def test_render_action_upcased
    assert_raise ActionView::MissingTemplate do
      get :render_action_upcased_hello_world
    end
  end

  # :ported:
  def test_render_action_hello_world_as_string
    get :render_action_hello_world_as_string
    assert_equal "Hello world!", @response.body