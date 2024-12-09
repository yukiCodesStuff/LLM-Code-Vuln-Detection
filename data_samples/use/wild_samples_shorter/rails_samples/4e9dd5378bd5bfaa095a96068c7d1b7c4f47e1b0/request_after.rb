
    # Override Rack's GET method to support indifferent access
    def GET
      @env["action_dispatch.request.query_parameters"] ||= deep_munge((normalize_encode_params(super) || {}))
    rescue TypeError => e
      raise ActionController::BadRequest.new(:query, e)
    end
    alias :query_parameters :GET

    # Override Rack's POST method to support indifferent access
    def POST
      @env["action_dispatch.request.request_parameters"] ||= deep_munge((normalize_encode_params(super) || {}))
    rescue TypeError => e
      raise ActionController::BadRequest.new(:request, e)
    end
    alias :request_parameters :POST