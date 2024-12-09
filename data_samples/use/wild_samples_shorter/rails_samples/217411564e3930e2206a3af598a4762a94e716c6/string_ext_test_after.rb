    assert_predicate string, :html_safe?
  end

  if "".respond_to?(:bytesplice)
    test "Bytesplicing safe into safe yields safe" do
      string = "hello".html_safe
      string.bytesplice(0, 0, "<b>".html_safe)

      assert_equal "<b>hello", string
      assert_predicate string, :html_safe?

      string = "hello".html_safe
      string.bytesplice(0..1, "<b>".html_safe)

      assert_equal "<b>llo", string
      assert_predicate string, :html_safe?
    end

    test "Bytesplicing unsafe into safe yields escaped safe" do
      string = "hello".html_safe
      string.bytesplice(1, 0, "<b>")

      assert_equal "h&lt;b&gt;ello", string
      assert_predicate string, :html_safe?

      string = "hello".html_safe
      string.bytesplice(1..2, "<b>")

      assert_equal "h&lt;b&gt;lo", string
      assert_predicate string, :html_safe?
    end
  end

  test "emits normal string YAML" do
    assert_equal "foo".to_yaml, "foo".html_safe.to_yaml(foo: 1)
  end
