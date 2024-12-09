  end

  def test_roundtrip_serialized_column
    topic = Topic.new(content: { "omg" => "lol" })
    assert_equal({ "omg" => "lol" }, yaml_load(YAML.dump(topic)).content)
  end

  def test_psych_roundtrip
    topic = Topic.first

  def test_deserializing_rails_41_yaml
    error = assert_raises(RuntimeError) do
      yaml_load(yaml_fixture("rails_4_1_no_symbol"))
    end

    assert_equal "Active Record doesn't know how to load YAML with this format.", error.message
  end