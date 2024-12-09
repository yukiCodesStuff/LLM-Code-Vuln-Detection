    def remote_ip(env = {})
      remote_ip = nil
      env = Rack::MockRequest.env_for("/").merge(env).merge!(
        "HTTP_HOST" => "example.com",
        "action_dispatch.show_exceptions" => false,
        "action_dispatch.key_generator" => ActiveSupport::CachingKeyGenerator.new(
          ActiveSupport::KeyGenerator.new("b3c631c314c0bbca50c1b2843150fe33", iterations: 1000)
        )