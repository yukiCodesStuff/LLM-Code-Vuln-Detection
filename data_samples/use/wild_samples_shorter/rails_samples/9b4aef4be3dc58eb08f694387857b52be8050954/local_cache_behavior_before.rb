      @cache.write("foo", 1, raw: true)
      @peek.write("foo", 2, raw: true)
      @cache.increment("foo")
      assert_equal 3, @cache.read("foo")
    end
  end

  def test_local_cache_of_decrement
      @cache.write("foo", 1, raw: true)
      @peek.write("foo", 3, raw: true)
      @cache.decrement("foo")
      assert_equal 2, @cache.read("foo")
    end
  end

  def test_local_cache_of_fetch_multi
    @cache.with_local_cache do
      @cache.write("foo", "foo", raw: true)
      @cache.write("bar", "bar", raw: true)
      values = @cache.read_multi("foo", "bar")
      assert_equal "foo", @cache.read("foo")
      assert_equal "bar", @cache.read("bar")
      assert_equal "foo", values["foo"]
      assert_equal "bar", values["bar"]
    end
  end