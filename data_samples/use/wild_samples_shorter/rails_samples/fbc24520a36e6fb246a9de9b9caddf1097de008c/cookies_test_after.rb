      head :ok
    end

    def set_cookie_with_domain_and_longer_tld
      cookies[:user_name] = { value: "rizwanreza", domain: :all, tld_length: 4 }
      head :ok 
    end

    def delete_cookie_with_domain_and_tld
      cookies.delete(:user_name, domain: :all, tld_length: 2)
      head :ok
    end
    assert_cookie_header "user_name=rizwanreza; domain=.nextangle.com.au; path=/; SameSite=Lax"
  end

  def test_cookie_with_all_domain_option_using_australian_style_tld_and_two_subdomains
    @request.host = "x.nextangle.com.au"
    get :set_cookie_with_domain
    assert_response :success
    assert_cookie_header "user_name=rizwanreza; domain=.nextangle.com.au; path=/; SameSite=Lax"
  end

  def test_cookie_with_all_domain_option_using_uk_style_tld
    @request.host = "nextangle.co.uk"
    get :set_cookie_with_domain
    assert_response :success
    assert_cookie_header "user_name=rizwanreza; domain=.nextangle.co.uk; path=/; SameSite=Lax"
  end

  def test_cookie_with_all_domain_option_using_uk_style_tld_and_two_subdomains
    @request.host = "x.nextangle.co.uk"
    get :set_cookie_with_domain
    assert_response :success
    assert_cookie_header "user_name=rizwanreza; domain=.nextangle.co.uk; path=/; SameSite=Lax"
  end

  def test_cookie_with_all_domain_option_using_host_with_port
    @request.host = "nextangle.local:3000"
    get :set_cookie_with_domain
    assert_response :success
    assert_cookie_header "user_name=rizwanreza; domain=.nextangle.local; path=/; SameSite=Lax"
  end

  def test_cookie_with_all_domain_option_using_longer_tld_length
    @request.host = "x.y.z.t.com"
    get :set_cookie_with_domain_and_longer_tld
    assert_response :success
    assert_cookie_header "user_name=rizwanreza; domain=.y.z.t.com; path=/; SameSite=Lax"
  end

  def test_deleting_cookie_with_all_domain_option_and_tld_length
    request.cookies[:user_name] = "Joe"
    get :delete_cookie_with_domain_and_tld
    assert_response :success