    assert_not defined?(@in_shared_context) # it's not in the test itself
  end

  private
    def call_and_return_body(&block)
      app = middleware(block || proc { [200, {}, "response"] })
      _, _, body = app.call("rack.input" => StringIO.new(""))