    end
  end

  def test_with_array_containing_symbols
    with_test_routes do
      assert_url "http://example.com/series/new", [:new, :series]
    end
    end
  end

  def test_string_route_arguments
    with_admin_test_routes do
      error = assert_raises(ArgumentError) do
        polymorphic_url(["admin", @project])
      end

      assert_equal("Please use symbols for polymorphic route arguments.", error.message)

      error = assert_raises(ArgumentError) do
        polymorphic_url([@project, "bid"])
      end

      assert_equal("Please use symbols for polymorphic route arguments.", error.message)
    end
  end

  def with_namespaced_routes(name)
    with_routing do |set|
      set.draw do
        scope(module: name) do