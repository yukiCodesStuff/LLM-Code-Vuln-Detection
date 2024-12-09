    assert_nil Edge.primary_key
  end

  def test_limit_with_comma
    assert_nothing_raised do
      Topic.limit("1,2").all
    end
  end

  def test_limit_without_comma
    assert_nothing_raised do
      assert_equal 1, Topic.limit("1").all.length
    end

    assert_nothing_raised do
      assert_equal 1, Topic.limit(1).all.length
    end
  end

  def test_invalid_limit
    assert_raises(ArgumentError) do
      Topic.limit("asdfadf").all
    end
  end

  def test_limit_should_sanitize_sql_injection_for_limit_without_comas
    assert_raises(ArgumentError) do
      Topic.limit("1 select * from schema").all
    end
  end

  def test_limit_should_sanitize_sql_injection_for_limit_with_comas
    assert_raises(ArgumentError) do
      Topic.limit("1, 7 procedure help()").all
    end
  end

  def test_select_symbol
    topic_ids = Topic.select(:id).map(&:id).sort
    assert_equal Topic.find(:all).map(&:id).sort, topic_ids
  end