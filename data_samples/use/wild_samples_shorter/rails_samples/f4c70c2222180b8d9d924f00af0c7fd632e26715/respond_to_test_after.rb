  def custom_type_handling
    respond_to do |type|
      type.html { render body: "HTML"    }
      type.custom("application/fancy-xml")  { render body: "Fancy XML"  }
      type.all  { render body: "Nothing" }
    end
  end

    @request.host = "www.example.com"
    Mime::Type.register_alias("text/html", :iphone)
    Mime::Type.register("text/x-mobile", :mobile)
    Mime::Type.register("application/fancy-xml", :fancy_xml)
  end

  def teardown
    super
    Mime::Type.unregister(:iphone)
    Mime::Type.unregister(:mobile)
    Mime::Type.unregister(:fancy_xml)
  end

  def test_html
    @request.accept = "text/html"
  end

  def test_custom_types
    @request.accept = "application/fancy-xml"
    get :custom_type_handling
    assert_equal "application/fancy-xml", @response.content_type
    assert_equal "Fancy XML", @response.body

    @request.accept = "text/html"
    get :custom_type_handling
    assert_equal "text/html", @response.content_type