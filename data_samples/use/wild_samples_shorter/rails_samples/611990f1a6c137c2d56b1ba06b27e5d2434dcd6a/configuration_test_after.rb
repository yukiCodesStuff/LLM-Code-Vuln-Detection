      assert_not ActiveRecord.suppress_multiple_database_warning
    end

    test "config.active_record.use_yaml_unsafe_load is false by default" do
      app "production"
      assert_not ActiveRecord.use_yaml_unsafe_load
    end

    test "config.active_record.use_yaml_unsafe_load can be configured" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/use_yaml_unsafe_load.rb", <<-RUBY
        Rails.application.config.active_record.use_yaml_unsafe_load = true
      RUBY

      app "production"
      assert ActiveRecord.use_yaml_unsafe_load
    end

    test "config.active_record.yaml_column_permitted_classes is [] by default" do
      app "production"
      assert_equal([], ActiveRecord.yaml_column_permitted_classes)
    end

    test "config.active_record.yaml_column_permitted_classes can be configured" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/yaml_permitted_classes.rb", <<-RUBY
        Rails.application.config.active_record.yaml_column_permitted_classes = [Symbol]
      RUBY

      app "production"
      assert_equal([Symbol], ActiveRecord.yaml_column_permitted_classes)
    end

    test "config.annotations wrapping SourceAnnotationExtractor::Annotation class" do
      make_basic_app do |application|
        application.config.annotations.register_extensions("coffee") do |tag|
          /#\s*(#{tag}):?\s*(.*)$/