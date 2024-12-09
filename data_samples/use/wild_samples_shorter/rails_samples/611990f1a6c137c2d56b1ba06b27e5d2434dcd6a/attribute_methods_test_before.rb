
  test "attribute_for_inspect with an array" do
    t = topics(:first)
    t.content = [Object.new]

    assert_match %r(\[#<Object:0x[0-9a-f]+>\]), t.attribute_for_inspect(:content)
  end

  test "attribute_for_inspect with a long array" do
    t = topics(:first)

  test "read attributes_for_database" do
    topic = Topic.new
    topic.content = { one: 1, two: 2 }

    db_attributes = Topic.instantiate(topic.attributes_for_database).attributes
    before_type_cast_attributes = Topic.instantiate(topic.attributes_before_type_cast).attributes

  end

  test "hashes are not mangled" do
    new_topic = { title: "New Topic", content: { key: "First value" } }
    new_topic_values = { title: "AnotherTopic", content: { key: "Second value" } }

    topic = Topic.new(new_topic)
    assert_equal new_topic[:title], topic.title
    assert_equal new_topic[:content], topic.content

    topic.attributes = new_topic_values
    assert_equal new_topic_values[:title], topic.title
    assert_equal new_topic_values[:content], topic.content
  end

  test "create through factory" do
    topic = Topic.create(title: "New Topic")
  end

  test "should unserialize attributes for frozen records" do
    myobj = { value1: :value2 }
    topic = Topic.create(content: myobj)
    topic.freeze
    assert_equal myobj, topic.content
  end