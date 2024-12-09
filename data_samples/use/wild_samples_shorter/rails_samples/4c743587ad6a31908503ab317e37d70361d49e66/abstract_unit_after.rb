      @app.config.session_store :cookie_store, key: "_myapp_session"
      @app.config.active_support.deprecation = :log
      @app.config.log_level = :info
      @app.secrets.secret_key_base = "b3c631c314c0bbca50c1b2843150fe33"

      yield @app if block_given?
      @app.initialize!
