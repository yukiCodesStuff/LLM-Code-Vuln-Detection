    testing RenderFile::BasicController

    test "rendering simple template" do
      assert_deprecated do
        get :index
      end
      assert_response "Hello world!"
    end

    test "rendering template with ivar" do
      assert_deprecated do
        get :with_instance_variables
      end
      assert_response "The secret is in the sauce\n"
    end

    test "rendering a relative path" do
      assert_deprecated do
        get :relative_path
      end
      assert_response "The secret is in the sauce\n"
    end

    test "rendering a relative path with dot" do
      assert_deprecated do
        get :relative_path_with_dot
      end
      assert_response "The secret is in the sauce\n"
    end

    test "rendering a Pathname" do
      assert_deprecated do
        get :pathname
      end
      assert_response "The secret is in the sauce\n"
    end

    test "rendering file with locals" do
      assert_deprecated do
        get :with_locals
      end
      assert_response "The secret is in the sauce\n"
    end
  end
end