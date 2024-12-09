            @prefix = prefix
            @suffix = suffix
            @parameters = parameters.nil? ? FORWARD_PARAMETERS : parameters
            @regex = /\A(?:#{Regexp.escape(@prefix)})(.*)(?:#{Regexp.escape(@suffix)})\z/
            @proxy_target = "#{@prefix}attribute#{@suffix}"
            @method_name = "#{prefix}%s#{suffix}"
          end
