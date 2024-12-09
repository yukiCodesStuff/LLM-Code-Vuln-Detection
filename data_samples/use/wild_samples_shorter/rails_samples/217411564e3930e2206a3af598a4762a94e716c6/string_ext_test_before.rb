    assert_predicate string, :html_safe?
  end

  test "emits normal string YAML" do
    assert_equal "foo".to_yaml, "foo".html_safe.to_yaml(foo: 1)
  end
