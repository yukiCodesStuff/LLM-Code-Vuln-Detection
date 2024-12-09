    redirect_to nil
  end

  def redirect_to_params
    redirect_to ActionController::Parameters.new(status: 200, protocol: "javascript", f: "%0Aeval(name)")
  end

    end
  end

  def test_redirect_to_nil
    error = assert_raise(ActionController::ActionControllerError) do
      get :redirect_to_nil
    end