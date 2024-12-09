    assert_not defined?(@in_shared_context) # it's not in the test itself
  end

  def test_body_abandonned
    total = 0
    ran = 0
    completed = 0

    executor.to_run { total += 1; ran += 1 }
    executor.to_complete { total += 1; completed += 1}

    stack = middleware(proc { [200, {}, "response"] })

    requests_count = 5

    requests_count.times do
      stack.call({})
    end

    assert_equal (requests_count * 2) - 1, total
    assert_equal requests_count, ran
    assert_equal requests_count - 1, completed
  end

  private
    def call_and_return_body(&block)
      app = middleware(block || proc { [200, {}, "response"] })
      _, _, body = app.call("rack.input" => StringIO.new(""))