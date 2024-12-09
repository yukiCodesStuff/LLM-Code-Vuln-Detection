    redirect_to "///www.rubyonrails.org/"
  end

  def only_path_redirect
    redirect_to action: "other_host", only_path: true
  end

    end
  end

  def test_only_path_redirect
    with_raise_on_open_redirects do
      get :only_path_redirect
      assert_response :redirect