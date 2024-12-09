    end
  end

  def test_register_and_use_yaml
    with_test_route_set do
      with_params_parsers Mime::YAML => Proc.new { |d| YAML.load(d) } do
        post "/", {"entry" => "loaded from yaml"}.to_yaml,