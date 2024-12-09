    # <tt>render :file => "foo/bar"</tt>.
    # :api: plugin
    def _normalize_args(action=nil, options={})
      if action.is_a? Hash
        action
      else
        options
      end