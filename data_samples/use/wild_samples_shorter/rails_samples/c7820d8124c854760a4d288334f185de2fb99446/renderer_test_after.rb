
  test "rendering with an instance renderer" do
    renderer = ApplicationController.renderer.new
    content  = assert_deprecated { renderer.render file: "test/hello_world" }

    assert_equal "Hello world!", content
  end
