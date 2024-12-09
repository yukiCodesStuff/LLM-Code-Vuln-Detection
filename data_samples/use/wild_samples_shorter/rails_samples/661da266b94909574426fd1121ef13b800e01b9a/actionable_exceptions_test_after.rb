      error: ActionError.name,
      action: "Successful action",
      location: "/",
    }, headers: { "action_dispatch.show_detailed_exceptions" => true }

    assert_equal ["Action!"], Actions

    assert_equal 302, response.status
      error: ActionError.name,
      action: "Successful action",
      location: "/",
    }, headers: { "action_dispatch.show_detailed_exceptions" => false }

    assert_empty Actions
  end

        error: ActionError.name,
        action: "Failed action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch non-actionable errors" do
        error: RuntimeError.name,
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end

  test "cannot dispatch Inexistent errors" do
        error: "",
        action: "Inexistent action",
        location: "/",
      }, headers: { "action_dispatch.show_detailed_exceptions" => true }
    end
  end
end