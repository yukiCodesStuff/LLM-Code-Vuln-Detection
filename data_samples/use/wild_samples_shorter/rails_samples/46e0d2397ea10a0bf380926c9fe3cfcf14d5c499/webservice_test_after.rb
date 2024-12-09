    end
  end

  def test_post_xml_using_a_disallowed_type_attribute
    $stderr = StringIO.new
    with_test_route_set do
      post '/', '<foo type="symbol">value</foo>', 'CONTENT_TYPE' => 'application/xml'
      assert_response 500

      post '/', '<foo type="yaml">value</foo>', 'CONTENT_TYPE' => 'application/xml'
      assert_response 500
    end
  ensure
    $stderr = STDERR
  end

  def test_register_and_use_yaml
    with_test_route_set do
      with_params_parsers Mime::YAML => Proc.new { |d| YAML.load(d) } do
        post "/", {"entry" => "loaded from yaml"}.to_yaml,