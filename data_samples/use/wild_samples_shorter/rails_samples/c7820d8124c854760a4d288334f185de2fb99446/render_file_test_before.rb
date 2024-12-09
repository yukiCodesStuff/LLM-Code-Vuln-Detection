    testing RenderFile::BasicController

    test "rendering simple template" do
      get :index
      assert_response "Hello world!"
    end

    test "rendering template with ivar" do
      get :with_instance_variables
      assert_response "The secret is in the sauce\n"
    end

    test "rendering a relative path" do
      get :relative_path
      assert_response "The secret is in the sauce\n"
    end

    test "rendering a relative path with dot" do
      get :relative_path_with_dot
      assert_response "The secret is in the sauce\n"
    end

    test "rendering a Pathname" do
      get :pathname
      assert_response "The secret is in the sauce\n"
    end

    test "rendering file with locals" do
      get :with_locals
      assert_response "The secret is in the sauce\n"
    end
  end
end