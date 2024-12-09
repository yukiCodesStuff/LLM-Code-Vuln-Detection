      assert_equal "some_value", verifier.verify(message)
    end

    test "raises when secret_key_base is blank" do
      app_file "config/initializers/secret_token.rb", <<-RUBY
        Rails.application.credentials.secret_key_base = nil
      RUBY

    test "application verifier can build different verifiers" do
      make_basic_app do |application|
        application.credentials.secret_key_base = "b3c631c314c0bbca50c1b2843150fe33"
        application.config.session_store :disabled
      end

      default_verifier = app.message_verifier(:sensitive_value)