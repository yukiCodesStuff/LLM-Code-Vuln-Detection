          email: {
            root: "#{Dir.tmpdir}/email",
            service: "Disk"
          }
        }
        config.action_mailbox.storage_service = :email
      RUBY
      app "development"
      assert_equal(:email, ActionMailbox.storage_service)
    end

    test "ActionMailer::Base.delivery_job is ActionMailer::MailDeliveryJob by default" do
      app "development"

      assert_equal ActionMailer::MailDeliveryJob, ActionMailer::Base.delivery_job
    end

    test "ActiveRecord::Base.filter_attributes should equal to filter_parameters" do
      app_file "config/initializers/filter_parameters_logging.rb", <<-RUBY
        Rails.application.config.filter_parameters += [ :password, :credit_card_number ]
      RUBY
      app "development"
      assert_equal [ :password, :credit_card_number ], Rails.application.config.filter_parameters
      assert_equal [ :password, :credit_card_number ], ActiveRecord::Base.filter_attributes
    end

    test "ActiveStorage.routes_prefix can be configured via config.active_storage.routes_prefix" do
      app_file "config/environments/development.rb", <<-RUBY
        Rails.application.configure do
          config.active_storage.routes_prefix = '/files'
        end
      RUBY

      output = rails("routes", "-g", "active_storage")
      assert_equal <<~MESSAGE, output
                               Prefix Verb URI Pattern                                                                        Controller#Action
                   rails_service_blob GET  /files/blobs/redirect/:signed_id/*filename(.:format)                               active_storage/blobs/redirect#show
             rails_service_blob_proxy GET  /files/blobs/proxy/:signed_id/*filename(.:format)                                  active_storage/blobs/proxy#show
                                      GET  /files/blobs/:signed_id/*filename(.:format)                                        active_storage/blobs/redirect#show
            rails_blob_representation GET  /files/representations/redirect/:signed_blob_id/:variation_key/*filename(.:format) active_storage/representations/redirect#show
      rails_blob_representation_proxy GET  /files/representations/proxy/:signed_blob_id/:variation_key/*filename(.:format)    active_storage/representations/proxy#show
                                      GET  /files/representations/:signed_blob_id/:variation_key/*filename(.:format)          active_storage/representations/redirect#show
                   rails_disk_service GET  /files/disk/:encoded_key/*filename(.:format)                                       active_storage/disk#show
            update_rails_disk_service PUT  /files/disk/:encoded_token(.:format)                                               active_storage/disk#update
                 rails_direct_uploads POST /files/direct_uploads(.:format)                                                    active_storage/direct_uploads#create
      MESSAGE
    end

    test "ActiveStorage.draw_routes can be configured via config.active_storage.draw_routes" do
      app_file "config/environments/development.rb", <<-RUBY
        Rails.application.configure do
          config.active_storage.draw_routes = false
        end
      RUBY

      output = rails("routes")
      assert_not_includes(output, "rails_service_blob")
      assert_not_includes(output, "rails_blob_representation")
      assert_not_includes(output, "rails_disk_service")
      assert_not_includes(output, "update_rails_disk_service")
      assert_not_includes(output, "rails_direct_uploads")
    end

    test "ActiveStorage.video_preview_arguments uses the old arguments without Rails 7 defaults" do
      remove_from_config '.*config\.load_defaults.*\n'

      app "development"

      assert_equal ActiveStorage.video_preview_arguments,
        "-y -vframes 1 -f image2"
    end

    test "ActiveStorage.video_preview_arguments uses the new arguments by default" do
      app "development"

      assert_equal ActiveStorage.video_preview_arguments,
        "-vf 'select=eq(n\\,0)+eq(key\\,1)+gt(scene\\,0.015),loop=loop=-1:size=2,trim=start_frame=1'" \
        " -frames:v 1 -f image2"
    end

    test "ActiveStorage.variant_processor uses mini_magick without Rails 7 defaults" do
      remove_from_config '.*config\.load_defaults.*\n'

      app "development"

      assert_equal :mini_magick, ActiveStorage.variant_processor
    end

    test "ActiveStorage.variant_processor uses vips by default" do
      app "development"

      assert_equal :vips, ActiveStorage.variant_processor
    end

    test "hosts include .localhost in development" do
      app "development"
      assert_includes Rails.application.config.hosts, ".localhost"
    end

    test "hosts reads multiple values from RAILS_DEVELOPMENT_HOSTS" do
      host = "agoodhost.com"
      another_host = "bananapants.com"
      switch_development_hosts_to(host, another_host) do
        app "development"
        assert_includes Rails.application.config.hosts, host
        assert_includes Rails.application.config.hosts, another_host
      end
    end

    test "hosts reads multiple values from RAILS_DEVELOPMENT_HOSTS and trims white space" do
      host = "agoodhost.com"
      host_with_white_space = "  #{host} "
      another_host = "bananapants.com"
      another_host_with_white_space = "     #{another_host}"
      switch_development_hosts_to(host_with_white_space, another_host_with_white_space) do
        app "development"
        assert_includes Rails.application.config.hosts, host
        assert_includes Rails.application.config.hosts, another_host
      end
    end

    test "hosts reads from RAILS_DEVELOPMENT_HOSTS" do
      host = "agoodhost.com"
      switch_development_hosts_to(host) do
        app "development"
        assert_includes Rails.application.config.hosts, host
      end
    end

    test "hosts does not read from RAILS_DEVELOPMENT_HOSTS in production" do
      host = "agoodhost.com"
      switch_development_hosts_to(host) do
        app "production"
        assert_not_includes Rails.application.config.hosts, host
      end
    end

    test "disable_sandbox is false by default" do
      app "development"

      assert_equal false, Rails.configuration.disable_sandbox
    end

    test "disable_sandbox can be overridden" do
      add_to_config <<-RUBY
        config.disable_sandbox = true
      RUBY

      app "development"

      assert Rails.configuration.disable_sandbox
    end

    test "rake_eager_load is false by default" do
      app "development"
      assert_equal false,  Rails.application.config.rake_eager_load
    end

    test "rake_eager_load is set correctly" do
      add_to_config <<-RUBY
        config.root = "#{app_path}"
        config.rake_eager_load = true
      RUBY

      app "development"

      assert_equal true, Rails.application.config.rake_eager_load
    end

    test "ActiveSupport::JsonWithMarshalFallback.fallback_to_marshal_deserialization is true by default" do
      app "development"

      assert_equal true, ActiveSupport::JsonWithMarshalFallback.fallback_to_marshal_deserialization
    end

    test "ActiveSupport::JsonWithMarshalFallback.fallback_to_marshal_deserialization is true by default for upgraded apps" do
      remove_from_config '.*config\.load_defaults.*\n'

      app "development"

      assert_equal true, ActiveSupport::JsonWithMarshalFallback.fallback_to_marshal_deserialization
    end

    test "ActiveSupport::JsonWithMarshalFallback.fallback_to_marshal_deserialization can be configured via config.active_support.fallback_to_marshal_deserialization" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/fallback_to_marshal_deserialization.rb", <<-RUBY
        Rails.application.config.active_support.fallback_to_marshal_deserialization = false
      RUBY

      app "development"

      assert_equal false, ActiveSupport::JsonWithMarshalFallback.fallback_to_marshal_deserialization
    end

    test "ActiveSupport::JsonWithMarshalFallback.use_marshal_serialization is true by default" do
      app "development"

      assert_equal true, ActiveSupport::JsonWithMarshalFallback.use_marshal_serialization
    end

    test "ActiveSupport::JsonWithMarshalFallback.use_marshal_serialization is true by default for upgraded apps" do
      remove_from_config '.*config\.load_defaults.*\n'

      app "development"

      assert_equal true, ActiveSupport::JsonWithMarshalFallback.use_marshal_serialization
    end

    test "ActiveSupport::JsonWithMarshalFallback.use_marshal_serialization can be configured via config.active_support.use_marshal_serialization" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/use_marshal_serialization.rb", <<-RUBY
        Rails.application.config.active_support.use_marshal_serialization = false
      RUBY

      app "development"

      assert_equal false, ActiveSupport::JsonWithMarshalFallback.use_marshal_serialization
    end

    test "ActiveSupport::MessageEncryptor.default_message_encryptor_serializer is :json by default" do
      app "development"

      assert_equal :json, ActiveSupport::MessageEncryptor.default_message_encryptor_serializer
    end

    test "ActiveSupport::MessageEncryptor.default_message_encryptor_serializer is :marshal by default for upgraded apps" do
      remove_from_config '.*config\.load_defaults.*\n'
      add_to_config 'config.load_defaults "6.1"'

      app "development"

      assert_equal :marshal, ActiveSupport::MessageEncryptor.default_message_encryptor_serializer
    end

    test "ActiveSupport::MessageEncryptor.default_message_encryptor_serializer can be configured via config.active_support.default_message_encryptor_serializer" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/default_message_encryptor_serializer.rb", <<-RUBY
        Rails.application.config.active_support.default_message_encryptor_serializer = :hybrid
      RUBY

      app "development"

      assert_equal :hybrid, ActiveSupport::MessageEncryptor.default_message_encryptor_serializer
    end

    test "ActiveSupport::MessageVerifier.default_message_verifier_serializer is :json by default for new apps" do
      app "development"

      assert_equal :json, ActiveSupport::MessageVerifier.default_message_verifier_serializer
    end

    test "ActiveSupport::MessageVerifier.default_message_verifier_serializer is :marshal by default for upgraded apps" do
      remove_from_config '.*config\.load_defaults.*\n'

      app "development"

      assert_equal :marshal, ActiveSupport::MessageVerifier.default_message_verifier_serializer
    end

    test "ActiveSupport::MessageVerifier.default_message_verifier_serializer can be configured via config.active_support.default_message_verifier_serializer" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/default_message_verifier_serializer.rb", <<-RUBY
        Rails.application.config.active_support.default_message_verifier_serializer = :hybrid
      RUBY

      app "development"

      assert_equal :hybrid, ActiveSupport::MessageVerifier.default_message_verifier_serializer
    end

    test "unknown_asset_fallback is false by default" do
      app "development"

      assert_equal false, Rails.application.config.assets.unknown_asset_fallback
    end

    test "legacy_connection_handling is false by default for new apps" do
      app "development"

      assert_equal false, Rails.application.config.active_record.legacy_connection_handling
    end

    test "legacy_connection_handling is not set before 6.1" do
      remove_from_config '.*config\.load_defaults.*\n'
      add_to_config 'config.load_defaults "6.0"'

      app "development"

      assert_nil Rails.application.config.active_record.legacy_connection_handling
    end

    test "ActionDispatch::Request.return_only_media_type_on_content_type is false by default" do
      app "development"

      assert_equal false, ActionDispatch::Request.return_only_media_type_on_content_type
    end

    test "ActionDispatch::Request.return_only_media_type_on_content_type is true in the 6.1 defaults" do
      remove_from_config '.*config\.load_defaults.*\n'
      add_to_config 'config.load_defaults "6.1"'

      app "development"

      assert_equal true, ActionDispatch::Request.return_only_media_type_on_content_type
    end

    test "ActionDispatch::Request.return_only_media_type_on_content_type can be configured in the new framework defaults" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/new_framework_defaults_7_0.rb", <<-RUBY
        Rails.application.config.action_dispatch.return_only_request_media_type_on_content_type = false
      RUBY

      app "development"

      assert_equal false, ActionDispatch::Request.return_only_media_type_on_content_type
    end

    test "action_dispatch.log_rescued_responses is true by default" do
      app "development"

      assert_equal true, Rails.application.env_config["action_dispatch.log_rescued_responses"]
    end

    test "action_dispatch.log_rescued_responses can be configured" do
      add_to_config <<-RUBY
        config.action_dispatch.log_rescued_responses = false
      RUBY

      app "development"

      assert_equal false, Rails.application.env_config["action_dispatch.log_rescued_responses"]
    end

    test "logs a warning when running SQLite3 in production" do
      restore_sqlite3_warning
      app_file "config/initializers/active_record.rb", <<~RUBY
        ActiveRecord::Base.establish_connection(adapter: "sqlite3", database: ":memory:")
      RUBY
      add_to_config "config.logger = MyLogRecorder.new"

      app "production"

      assert_match(/You are running SQLite in production, this is generally not recommended/, Rails.logger.recording)
    end

    test "doesn't log a warning when running SQLite3 in production and sqlite3_production_warning=false" do
      app_file "config/initializers/active_record.rb", <<~RUBY
        ActiveRecord::Base.establish_connection(adapter: "sqlite3", database: ":memory:")
      RUBY
      add_to_config "config.logger = MyLogRecorder.new"

      app "production"

      assert_no_match(/You are running SQLite in production, this is generally not recommended/, Rails.logger.recording)
    end

    test "doesn't log a warning when running MySQL in production" do
      restore_sqlite3_warning
      original_configurations = ActiveRecord::Base.configurations
      ActiveRecord::Base.configurations = { production: { db1: { adapter: "mysql2" } } }
      app_file "config/initializers/active_record.rb", <<~RUBY
        ActiveRecord::Base.establish_connection(adapter: "mysql2")
      RUBY
      add_to_config "config.logger = MyLogRecorder.new"

      app "production"

      assert_no_match(/You are running SQLite in production, this is generally not recommended/, Rails.logger.recording)
    ensure
      ActiveRecord::Base.configurations = original_configurations
    end

    test "doesn't log a warning when running SQLite3 in development" do
      restore_sqlite3_warning
      app_file "config/initializers/active_record.rb", <<~RUBY
        ActiveRecord::Base.establish_connection(adapter: "sqlite3", database: ":memory:")
      RUBY
      add_to_config "config.logger = MyLogRecorder.new"

      app "development"

      assert_no_match(/You are running SQLite in production, this is generally not recommended/, Rails.logger.recording)
    end

    test "app starts with LocalCache middleware" do
      app "development"

      assert(Rails.application.config.middleware.map(&:name).include?("ActiveSupport::Cache::Strategy::LocalCache"))

      local_cache_index = Rails.application.config.middleware.map(&:name).index("ActiveSupport::Cache::Strategy::LocalCache")
      logger_index = Rails.application.config.middleware.map(&:name).index("Rails::Rack::Logger")
      assert local_cache_index < logger_index
    end

    test "LocalCache middleware can be moved via app config" do
      # you can't move Rails.cache.middleware as it doesn't exist yet
      add_to_config "config.middleware.move_after(Rails::Rack::Logger, ActiveSupport::Cache::Strategy::LocalCache)"

      app "development"

      local_cache_index = Rails.application.config.middleware.map(&:name).index("ActiveSupport::Cache::Strategy::LocalCache")
      logger_index = Rails.application.config.middleware.map(&:name).index("Rails::Rack::Logger")
      assert local_cache_index > logger_index
    end

    test "LocalCache middleware can be moved via initializer" do
      app_file "config/initializers/move_local_cache_middleware.rb", <<~RUBY
        Rails.application.config.middleware.move_after(Rails::Rack::Logger, Rails.cache.middleware)
      RUBY

      app "development"

      local_cache_index = Rails.application.config.middleware.map(&:name).index("ActiveSupport::Cache::Strategy::LocalCache")
      logger_index = Rails.application.config.middleware.map(&:name).index("Rails::Rack::Logger")
      assert local_cache_index > logger_index
    end

    test "LocalCache middleware can be removed via app config" do
      # you can't delete Rails.cache.middleware as it doesn't exist yet
      add_to_config "config.middleware.delete(ActiveSupport::Cache::Strategy::LocalCache)"

      app "development"

      assert_not(Rails.application.config.middleware.map(&:name).include?("ActiveSupport::Cache::Strategy::LocalCache"))
    end

    test "LocalCache middleware can be removed via initializer" do
      app_file "config/initializers/remove_local_cache_middleware.rb", <<~RUBY
        Rails.application.config.middleware.delete(Rails.cache.middleware)
      RUBY

      app "development"

      assert_not(Rails.application.config.middleware.map(&:name).include?("ActiveSupport::Cache::Strategy::LocalCache"))
    end

    test "custom middleware with overridden names can be added, moved, or deleted" do
      app_file "config/initializers/add_custom_middleware.rb", <<~RUBY
        class CustomMiddlewareOne
          def self.name
            "1st custom middleware"
          end
          def initialize(app, *args); end
          def new(app); self; end
        end

        class CustomMiddlewareTwo
          def initialize(app, *args); end
          def new(app); self; end
        end

        class CustomMiddlewareThree
          def self.name
            "3rd custom middleware"
          end
          def initialize(app, *args); end
          def new(app); self; end
        end

        Rails.application.config.middleware.use(CustomMiddlewareOne)
        Rails.application.config.middleware.use(CustomMiddlewareTwo)
        Rails.application.config.middleware.use(CustomMiddlewareThree)
        Rails.application.config.middleware.move_after(CustomMiddlewareTwo, CustomMiddlewareOne)
        Rails.application.config.middleware.delete(CustomMiddlewareThree)
      RUBY

      app "development"

      custom_middleware_one = Rails.application.config.middleware.map(&:name).index("1st custom middleware")
      custom_middleware_two = Rails.application.config.middleware.map(&:name).index("CustomMiddlewareTwo")
      assert custom_middleware_one > custom_middleware_two

      assert_nil Rails.application.config.middleware.map(&:name).index("3rd custom middleware")
    end

    test "ActiveSupport::TimeWithZone.name uses default Ruby implementation by default" do
      app "development"
      assert_equal false, ActiveSupport::TimeWithZone.methods(false).include?(:name)
    end

    test "ActiveSupport::TimeWithZone.name can be configured in the new framework defaults" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/new_framework_defaults_7_0.rb", <<-RUBY
        Rails.application.config.active_support.remove_deprecated_time_with_zone_name = false
      RUBY

      app "development"

      assert_equal true, ActiveSupport::TimeWithZone.methods(false).include?(:name)
    end

    test "can entirely opt out of ActiveSupport::Deprecations" do
      add_to_config "config.active_support.report_deprecations = false"

      app "production"

      assert_equal true, ActiveSupport::Deprecation.silenced
      assert_equal [ActiveSupport::Deprecation::DEFAULT_BEHAVIORS[:silence]], ActiveSupport::Deprecation.behavior
      assert_equal [ActiveSupport::Deprecation::DEFAULT_BEHAVIORS[:silence]], ActiveSupport::Deprecation.disallowed_behavior
    end

    test "ParamsWrapper is enabled in a new app and uses JSON as the format" do
      app "production"

      assert_equal [:json], ActionController::Base._wrapper_options.format
    end

    test "ParamsWrapper is enabled in an upgrade and uses JSON as the format" do
      remove_from_config '.*config\.load_defaults.*\n'
      add_to_config 'config.load_defaults "6.1"'

      app_file "config/initializers/new_framework_defaults_7_0.rb", <<-RUBY
        Rails.application.config.action_controller.wrap_parameters_by_default = true
      RUBY

      app "production"

      assert_equal [:json], ActionController::Base._wrapper_options.format
    end

    test "ParamsWrapper can be changed from the default in the initializer that was created prior to Rails 7" do
      app_file "config/initializers/wrap_parameters.rb", <<-RUBY
        ActiveSupport.on_load(:action_controller) do
          wrap_parameters format: [:xml]
        end
      RUBY

      app "production"

      assert_equal [:xml], ActionController::Base._wrapper_options.format
    end

    test "ParamsWrapper can be turned off" do
      add_to_config "Rails.application.config.action_controller.wrap_parameters_by_default = false"

      app "production"

      assert_equal [], ActionController::Base._wrapper_options.format
    end

    test "deprecated #to_s with format works with the Rails 6.1 defaults" do
      remove_from_config '.*config\.load_defaults.*\n'
      add_to_config 'config.load_defaults "6.1"'

      app "production"

      assert_deprecated do
        assert_equal "21 Feb", Date.new(2005, 2, 21).to_s(:short)
      end
      assert_deprecated do
        assert_equal "2005-02-21 14:30:00", DateTime.new(2005, 2, 21, 14, 30, 0, 0).to_s(:db)
      end
      assert_deprecated do
        assert_equal "555-1234", 5551234.to_s(:phone)
      end
      assert_deprecated do
        assert_equal "BETWEEN 'a' AND 'z'", ("a".."z").to_s(:db)
      end
      assert_deprecated do
        assert_equal "2005-02-21 17:44:30", Time.utc(2005, 2, 21, 17, 44, 30.12345678901).to_s(:db)
      end
    end

    test "deprecated #to_s with format does not work with the Rails 6.1 defaults and the config set" do
      remove_from_config '.*config\.load_defaults.*\n'
      add_to_config 'config.load_defaults "6.1"'

      add_to_config <<-RUBY
        config.active_support.disable_to_s_conversion = true
      RUBY

      app "production"

      assert_raises(ArgumentError) do
        Date.new(2005, 2, 21).to_s(:short)
      end
      assert_raises(ArgumentError) do
        DateTime.new(2005, 2, 21, 14, 30, 0, 0).to_s(:db)
      end
      assert_raises(TypeError) do
        5551234.to_s(:phone)
      end
      assert_raises(ArgumentError) do
        ("a".."z").to_s(:db)
      end
      assert_raises(ArgumentError) do
        Time.utc(2005, 2, 21, 17, 44, 30.12345678901).to_s(:db)
      end
    end

    test "deprecated #to_s with format does not work with the Rails 7.0 defaults" do
      app "production"

      assert_raises(ArgumentError) do
        Date.new(2005, 2, 21).to_s(:short)
      end
      assert_raises(ArgumentError) do
        DateTime.new(2005, 2, 21, 14, 30, 0, 0).to_s(:db)
      end
      assert_raises(TypeError) do
        5551234.to_s(:phone)
      end
      assert_raises(ArgumentError) do
        ("a".."z").to_s(:db)
      end
      assert_raises(ArgumentError) do
        Time.utc(2005, 2, 21, 17, 44, 30.12345678901).to_s(:db)
      end
    end

    test "ActionController::Base.raise_on_missing_callback_actions is false by default for production" do
      app "production"

      assert_equal false, ActionController::Base.raise_on_missing_callback_actions
    end

    test "ActionController::Base.raise_on_missing_callback_actions is false by default for upgraded apps" do
      remove_from_config '.*config\.load_defaults.*\n'

      app "development"

      assert_equal false, ActionController::Base.raise_on_missing_callback_actions
    end

    test "ActionController::Base.raise_on_missing_callback_actions can be configured in the new framework defaults" do
      remove_from_config '.*config\.load_defaults.*\n'

      app_file "config/initializers/new_framework_defaults_6_2.rb", <<-RUBY
        Rails.application.config.action_controller.raise_on_missing_callback_actions = true
      RUBY

      app "production"

      assert_equal true, ActionController::Base.raise_on_missing_callback_actions
    end

    private
      def set_custom_config(contents, config_source = "custom".inspect)
        app_file "config/custom.yml", contents

        add_to_config <<~RUBY
          config.my_custom_config = config_for(#{config_source})
        RUBY
      end
  end
end