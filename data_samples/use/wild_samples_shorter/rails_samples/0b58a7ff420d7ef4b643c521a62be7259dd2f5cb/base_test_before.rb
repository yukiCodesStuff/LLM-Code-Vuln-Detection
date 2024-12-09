    assert_nil Edge.primary_key
  end

  def test_select_symbol
    topic_ids = Topic.select(:id).map(&:id).sort
    assert_equal Topic.find(:all).map(&:id).sort, topic_ids
  end