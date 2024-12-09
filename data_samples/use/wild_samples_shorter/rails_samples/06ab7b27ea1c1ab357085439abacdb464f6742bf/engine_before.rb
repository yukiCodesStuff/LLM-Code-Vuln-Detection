      text/xml
      application/xml
      application/xhtml+xml
    )

    config.eager_load_namespaces << ActiveStorage

        ActiveStorage.variable_content_types = app.config.active_storage.variable_content_types || []
        ActiveStorage.content_types_to_serve_as_binary = app.config.active_storage.content_types_to_serve_as_binary || []
        ActiveStorage.service_urls_expire_in = app.config.active_storage.service_urls_expire_in || 5.minutes
      end
    end

    initializer "active_storage.attached" do