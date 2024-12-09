      assert_not ActiveRecord.suppress_multiple_database_warning
    end

    test "config.annotations wrapping SourceAnnotationExtractor::Annotation class" do
      make_basic_app do |application|
        application.config.annotations.register_extensions("coffee") do |tag|
          /#\s*(#{tag}):?\s*(.*)$/