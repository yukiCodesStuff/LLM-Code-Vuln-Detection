      head :ok
    end
  end
  class EarlyParse
    def initialize(app)
      @app = app
    end

    def call(env)
      # Trigger a Rack parse so that env caches the query params
      Rack::Request.new(env).params
      @app.call(env)
    end
  end

  def teardown
    TestController.last_query_parameters = nil
  end
        set.draw do
          get ':action', :to => ::QueryStringParsingTest::TestController
        end
        @app = self.class.build_app(set) do |middleware|
          middleware.use(EarlyParse)
        end


        get "/parse", actual
        assert_response :ok
        assert_equal(expected, ::QueryStringParsingTest::TestController.last_query_parameters)