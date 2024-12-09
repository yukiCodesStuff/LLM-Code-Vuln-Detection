    redirect_to nil
  end

  def redirect_to_polymorphic
    redirect_to [:internal, Workshop.new(5)]
  end

  def redirect_to_polymorphic_string_args
    redirect_to ["internal", Workshop.new(5)]
  end

  def redirect_to_params
    redirect_to ActionController::Parameters.new(status: 200, protocol: "javascript", f: "%0Aeval(name)")
  end

    end
  end

  def test_polymorphic_redirect
    with_routing do |set|
      set.draw do
        namespace :internal do
          resources :workshops
        end

        ActiveSupport::Deprecation.silence do
          get ":controller/:action"
        end
      end

      get :redirect_to_polymorphic
      assert_equal "http://test.host/internal/workshops/5", redirect_to_url
      assert_redirected_to [:internal, Workshop.new(5)]
    end
  end

  def test_polymorphic_redirect_with_string_args
    with_routing do |set|
      set.draw do
        namespace :internal do
          resources :workshops
        end

        ActiveSupport::Deprecation.silence do
          get ":controller/:action"
        end
      end

      error = assert_raises(ArgumentError) do
        get :redirect_to_polymorphic_string_args
      end
      assert_equal("Please use symbols for polymorphic route arguments.", error.message)
    end
  end

  def test_redirect_to_nil
    error = assert_raise(ActionController::ActionControllerError) do
      get :redirect_to_nil
    end