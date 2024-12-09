
  def test_save_should_store_serialized_attributes_even_with_partial_writes
    with_partial_writes(Topic) do
      topic = Topic.create!(content: { "a" => "a" })

      assert_not_predicate topic, :changed?

      topic.content["b"] = "b"

      assert_predicate topic, :changed?

      topic.save!

      assert_not_predicate topic, :changed?
      assert_equal "b", topic.content["b"]

      topic.reload

      assert_equal "b", topic.content["b"]
    end
  end

  def test_save_always_should_update_timestamps_when_serialized_attributes_are_present
    with_partial_writes(Topic) do
      topic = Topic.create!(content: { "a" => "a" })
      topic.save!

      updated_at = topic.updated_at
      travel(1.second) do
        topic.content["hello"] = "world"
        topic.save!
      end

      assert_not_equal updated_at, topic.updated_at
      assert_equal "world", topic.content["hello"]
    end
  end

  def test_save_should_not_save_serialized_attribute_with_partial_writes_if_not_present
    with_partial_writes(Topic) do
      topic = Topic.create!(author_name: "Bill", content: { "a" => "a" })
      topic = Topic.select("id, author_name").find(topic.id)
      topic.update_columns author_name: "John"
      assert_not_nil topic.reload.content
    end
  end

  def test_changes_to_save_should_not_mutate_array_of_hashes
    topic = Topic.new(author_name: "Bill", content: [{ "a" => "a" }])

    topic.changes_to_save

    assert_equal [{ "a" => "a" }], topic.content
  end

  def test_previous_changes
    # original values should be in previous_changes