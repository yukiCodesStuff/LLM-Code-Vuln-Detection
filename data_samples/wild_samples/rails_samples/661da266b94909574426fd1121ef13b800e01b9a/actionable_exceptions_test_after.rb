    post ActionDispatch::ActionableExceptions.endpoint, params: {
      error: ActionError.name,
      action: "Successful action",
      location: "/",
    }, headers: { "action_dispatch.show_detailed_exceptions" => true }

    assert_equal ["Action!"], Actions

    assert_equal 302, response.status
    assert_equal "/", response.headers["Location"]
  end

  test "cannot dispatch errors if not allowed" do
    post ActionDispatch::ActionableExceptions.endpoint, params: {
      error: ActionError.name,
      action: "Successful action",
      location: "/",
    }, headers: { "action_dispatch.show_detailed_exceptions" => false }

    assert_empty Actions
  end

  test "dispatched action can fail" do
    assert_raise RuntimeError do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: ActionError.name,
        action: "Failed action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch non-actionable errors" do
    assert_raise ActiveSupport::ActionableError::NonActionable do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: RuntimeError.name,
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch Inexistent errors" do
    assert_raise ActiveSupport::ActionableError::NonActionable do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: "",
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end
end
    post ActionDispatch::ActionableExceptions.endpoint, params: {
      error: ActionError.name,
      action: "Successful action",
      location: "/",
    }, headers: { "action_dispatch.show_detailed_exceptions" => false }

    assert_empty Actions
  end

  test "dispatched action can fail" do
    assert_raise RuntimeError do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: ActionError.name,
        action: "Failed action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch non-actionable errors" do
    assert_raise ActiveSupport::ActionableError::NonActionable do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: RuntimeError.name,
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch Inexistent errors" do
    assert_raise ActiveSupport::ActionableError::NonActionable do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: "",
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end
end
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: ActionError.name,
        action: "Failed action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch non-actionable errors" do
    assert_raise ActiveSupport::ActionableError::NonActionable do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: RuntimeError.name,
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch Inexistent errors" do
    assert_raise ActiveSupport::ActionableError::NonActionable do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: "",
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end
end
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: RuntimeError.name,
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch Inexistent errors" do
    assert_raise ActiveSupport::ActionableError::NonActionable do
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: "",
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end
end
      post ActionDispatch::ActionableExceptions.endpoint, params: {
        error: "",
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end
end