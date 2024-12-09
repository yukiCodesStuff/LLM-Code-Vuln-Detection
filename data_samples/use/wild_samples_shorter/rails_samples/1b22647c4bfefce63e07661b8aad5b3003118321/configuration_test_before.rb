      assert ActiveRecord.use_yaml_unsafe_load
    end

    test "config.active_record.yaml_column_permitted_classes is [Symbol] by default" do
      app "production"
      assert_equal([Symbol], ActiveRecord.yaml_column_permitted_classes)
    end