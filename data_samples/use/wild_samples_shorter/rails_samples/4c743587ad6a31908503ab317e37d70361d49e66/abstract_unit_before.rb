      @app.config.session_store :cookie_store, key: "_myapp_session"
      @app.config.active_support.deprecation = :log
      @app.config.log_level = :info

      yield @app if block_given?
      @app.initialize!
