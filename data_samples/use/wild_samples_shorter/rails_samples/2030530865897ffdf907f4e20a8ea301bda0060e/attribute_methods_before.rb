            @prefix = prefix
            @suffix = suffix
            @parameters = parameters.nil? ? FORWARD_PARAMETERS : parameters
            @regex = /^(?:#{Regexp.escape(@prefix)})(.*)(?:#{Regexp.escape(@suffix)})$/
            @proxy_target = "#{@prefix}attribute#{@suffix}"
            @method_name = "#{prefix}%s#{suffix}"
          end
