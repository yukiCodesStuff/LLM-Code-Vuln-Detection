      elsif options.key?(:html)
        Template::HTML.new(options[:html], formats.first)
      elsif options.key?(:file)
        with_fallbacks { find_file(options[:file], nil, false, keys, @details) }
      elsif options.key?(:inline)
        handler = Template.handler_for_extension(options[:type] || "erb")
        Template.new(options[:inline], "inline template", handler, :locals => keys)
      elsif options.key?(:template)