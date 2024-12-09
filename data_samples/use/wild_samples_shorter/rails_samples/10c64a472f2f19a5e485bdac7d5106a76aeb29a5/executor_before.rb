    end

    def call(env)
      state = @executor.run!
      begin
        response = @app.call(env)
        returned = response << ::Rack::BodyProxy.new(response.pop) { state.complete! }
      rescue => error