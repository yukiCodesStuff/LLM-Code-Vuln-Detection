            Template::File.new(options[:file])
          else
            ActiveSupport::Deprecation.warn "render file: should be given the absolute path to a file"
            @lookup_context.with_fallbacks.find_file(options[:file], nil, false, keys, @details)
          end
        elsif options.key?(:inline)
          handler = Template.handler_for_extension(options[:type] || "erb")
          format = if handler.respond_to?(:default_format)