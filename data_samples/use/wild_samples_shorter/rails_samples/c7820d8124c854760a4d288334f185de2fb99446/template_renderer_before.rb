        elsif options.key?(:html)
          Template::HTML.new(options[:html], formats.first)
        elsif options.key?(:file)
          @lookup_context.with_fallbacks.find_file(options[:file], nil, false, keys, @details)
        elsif options.key?(:inline)
          handler = Template.handler_for_extension(options[:type] || "erb")
          format = if handler.respond_to?(:default_format)
            handler.default_format