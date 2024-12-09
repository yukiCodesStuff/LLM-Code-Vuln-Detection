  end

  def test_pluck_with_serialization
    t = Topic.create!(content: { foo: :bar })
    assert_equal [{ foo: :bar }], Topic.where(id: t.id).pluck(:content)
  end

  def test_pluck_with_qualified_column_name
    assert_equal [1, 2, 3, 4, 5], Topic.order(:id).pluck("topics.id")