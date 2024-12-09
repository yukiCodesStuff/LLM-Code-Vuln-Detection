    end
  end

  def test_with_array_containing_single_string_name
    with_test_routes do
      assert_url "http://example.com/projects", ["projects"]
    end
  end

  def test_with_array_containing_symbols
    with_test_routes do
      assert_url "http://example.com/series/new", [:new, :series]
    end
    end
  end

  def with_namespaced_routes(name)
    with_routing do |set|
      set.draw do
        scope(module: name) do