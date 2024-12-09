      assert_equal :vips, ActiveStorage.variant_processor
    end

    test "ActiveStorage.supported_image_processing_methods can be configured via config.active_storage.supported_image_processing_methods" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/add_image_processing_methods.rb", <<-RUBY
        Rails.application.config.active_storage.supported_image_processing_methods = ["write", "set"]
      RUBY

      app "development"

      assert ActiveStorage.supported_image_processing_methods.include?("write")
      assert ActiveStorage.supported_image_processing_methods.include?("set")
    end

    test "ActiveStorage.unsupported_image_processing_arguments can be configured via config.active_storage.unsupported_image_processing_arguments" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/add_image_processing_arguments.rb", <<-RUBY
      Rails.application.config.active_storage.unsupported_image_processing_arguments = %w(
        -write
        -danger
      )
      RUBY

      app "development"

      assert ActiveStorage.unsupported_image_processing_arguments.include?("-danger")
      refute ActiveStorage.unsupported_image_processing_arguments.include?("-set")
    end

    test "hosts include .localhost in development" do
      app "development"
      assert_includes Rails.application.config.hosts, ".localhost"
    end