
  def test_incomplete_schema_loading
    topic = Topic.first
    payload = { foo: 42 }
    topic.update!(content: payload)

    Topic.reset_column_information
