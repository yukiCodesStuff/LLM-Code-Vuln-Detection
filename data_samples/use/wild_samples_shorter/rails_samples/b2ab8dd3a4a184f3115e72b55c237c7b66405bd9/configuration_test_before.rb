      assert_equal :vips, ActiveStorage.variant_processor
    end

    test "hosts include .localhost in development" do
      app "development"
      assert_includes Rails.application.config.hosts, ".localhost"
    end