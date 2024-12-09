      assert ActiveRecord.use_yaml_unsafe_load
    end

    test "config.active_record.raise_int_wider_than_64bit is true by default" do
      app "production"
      assert ActiveRecord.raise_int_wider_than_64bit
    end

    test "config.active_record.raise_int_wider_than_64bit can be configured" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/dont_raise.rb", <<-RUBY
        Rails.application.config.active_record.raise_int_wider_than_64bit = false
      RUBY

      app "production"
      assert_not ActiveRecord.raise_int_wider_than_64bit
    end


    test "config.active_record.yaml_column_permitted_classes is [Symbol] by default" do
      app "production"
      assert_equal([Symbol], ActiveRecord.yaml_column_permitted_classes)
    end