      error: ActionError.name,
      action: "Successful action",
      location: "/",
    }

    assert_equal ["Action!"], Actions

    assert_equal 302, response.status
      error: ActionError.name,
      action: "Successful action",
      location: "/",
    }, headers: { "action_dispatch.show_exceptions" => false }

    assert_empty Actions
  end

        error: ActionError.name,
        action: "Failed action",
        location: "/",
      }
    end
  end

  test "cannot dispatch non-actionable errors" do
        error: RuntimeError.name,
        action: "Inexistent action",
        location: "/",
      }
    end
  end

  test "cannot dispatch Inexistent errors" do
        error: "",
        action: "Inexistent action",
        location: "/",
      }
    end
  end
end