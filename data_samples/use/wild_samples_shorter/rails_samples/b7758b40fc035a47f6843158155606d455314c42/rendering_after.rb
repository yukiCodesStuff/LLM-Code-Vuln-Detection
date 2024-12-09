    # <tt>render :file => "foo/bar"</tt>.
    # :api: plugin
    def _normalize_args(action=nil, options={})
      case action
      when ActionController::Parameters
        unless action.permitted?
          raise ArgumentError, "render parameters are not permitted"
        end
        action
      when Hash
        action
      else
        options
      end