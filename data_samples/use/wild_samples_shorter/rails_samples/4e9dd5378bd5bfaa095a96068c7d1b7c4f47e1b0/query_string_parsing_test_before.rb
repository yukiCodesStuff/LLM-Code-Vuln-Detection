      head :ok
    end
  end

  def teardown
    TestController.last_query_parameters = nil
  end
        set.draw do
          get ':action', :to => ::QueryStringParsingTest::TestController
        end

        get "/parse", actual
        assert_response :ok
        assert_equal(expected, ::QueryStringParsingTest::TestController.last_query_parameters)