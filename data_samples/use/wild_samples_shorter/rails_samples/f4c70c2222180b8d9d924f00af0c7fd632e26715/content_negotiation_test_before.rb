      assert_body "Hello world */*!"
    end

    test "Not all mimes are converted to symbol" do
      get "/content_negotiation/basic/all", headers: { "HTTP_ACCEPT" => "text/plain, mime/another" }
      assert_body '[:text, "mime/another"]'
    end
  end
end