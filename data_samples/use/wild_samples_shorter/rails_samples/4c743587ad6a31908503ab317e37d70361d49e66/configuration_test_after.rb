      assert_equal "some_value", verifier.verify(message)
    end

    test "application will generate secret_key_base in tmp file if blank in development" do
      app_file "config/initializers/secret_token.rb", <<-RUBY
        Rails.application.credentials.secret_key_base = nil
      RUBY

      app "development"

      assert_not_nil app.secrets.secret_key_base
      assert File.exist?(app_path("tmp/development_secret.txt"))
    end

    test "application will not generate secret_key_base in tmp file if blank in production" do
      app_file "config/initializers/secret_token.rb", <<-RUBY
        Rails.application.credentials.secret_key_base = nil
      RUBY

      assert_raises ArgumentError do
        app "production"
      end
    end

    test "raises when secret_key_base is blank" do
      app_file "config/initializers/secret_token.rb", <<-RUBY
        Rails.application.credentials.secret_key_base = nil
      RUBY

    test "application verifier can build different verifiers" do
      make_basic_app do |application|
        application.config.session_store :disabled
      end

      default_verifier = app.message_verifier(:sensitive_value)