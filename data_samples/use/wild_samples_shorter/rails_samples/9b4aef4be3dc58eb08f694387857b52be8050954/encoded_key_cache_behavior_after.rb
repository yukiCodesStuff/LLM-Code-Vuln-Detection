    define_method "test_#{encoding.name.underscore}_encoded_values" do
      key = (+"foo").force_encoding(encoding)
      assert @cache.write(key, "1", raw: true)
      assert_equal "1", @cache.read(key, raw: true)
      assert_equal "1", @cache.fetch(key, raw: true)
      assert @cache.delete(key)
      assert_equal "2", @cache.fetch(key, raw: true) { "2" }
      assert_equal 3, @cache.increment(key)
      assert_equal 2, @cache.decrement(key)
  def test_common_utf8_values
    key = (+"\xC3\xBCmlaut").force_encoding(Encoding::UTF_8)
    assert @cache.write(key, "1", raw: true)
    assert_equal "1", @cache.read(key, raw: true)
    assert_equal "1", @cache.fetch(key, raw: true)
    assert @cache.delete(key)
    assert_equal "2", @cache.fetch(key, raw: true) { "2" }
    assert_equal 3, @cache.increment(key)
    assert_equal 2, @cache.decrement(key)