      application/pdf
    )

    config.eager_load_namespaces << ActiveStorage

    initializer "active_storage.configs" do
      config.after_initialize do |app|
        ActiveStorage.draw_routes       = app.config.active_storage.draw_routes != false
        ActiveStorage.resolve_model_to_route = app.config.active_storage.resolve_model_to_route || :rails_storage_redirect

        ActiveStorage.variable_content_types = app.config.active_storage.variable_content_types || []
        ActiveStorage.web_image_content_types = app.config.active_storage.web_image_content_types || []
        ActiveStorage.content_types_to_serve_as_binary = app.config.active_storage.content_types_to_serve_as_binary || []
        ActiveStorage.service_urls_expire_in = app.config.active_storage.service_urls_expire_in || 5.minutes