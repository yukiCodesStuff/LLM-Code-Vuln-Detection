      LOCALHOST =~ remote_addr && LOCALHOST =~ remote_ip
    end

    private

    def check_method(name)
      HTTP_METHOD_LOOKUP[name] || raise(ActionController::UnknownHttpMethod, "#{name}, accepted HTTP methods are #{HTTP_METHODS.to_sentence(:locale => :en)}")