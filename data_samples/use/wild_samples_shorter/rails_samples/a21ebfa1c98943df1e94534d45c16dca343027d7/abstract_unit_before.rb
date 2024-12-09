    end

    def get(path)
      @app.call(::Rack::MockRequest.env_for(path, "HTTP_HOST" => "example.com"))
    end

    def assert_welcome(resp)
      resp = Array(resp)