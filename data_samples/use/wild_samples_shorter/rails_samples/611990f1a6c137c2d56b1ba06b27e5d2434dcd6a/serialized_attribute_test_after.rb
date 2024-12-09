require "models/binary_field"

class SerializedAttributeTest < ActiveRecord::TestCase
  def setup
    ActiveRecord.use_yaml_unsafe_load = true
  end

  fixtures :topics, :posts

  MyObject = Struct.new :attribute1, :attribute2


    klass = Class.new(ActiveRecord::Base) do
      self.table_name = Topic.table_name
      store :content, coder: ActiveRecord::Coders::JSON
      attribute :content, :encrypted, subtype: type_for_attribute(:content)
    end

    topic = klass.create!(content: { trial: true })
  def test_decorated_type_with_decorator_block
    klass = Class.new(ActiveRecord::Base) do
      self.table_name = Topic.table_name
      store :content, coder: ActiveRecord::Coders::JSON
      attribute(:content) { |subtype| EncryptedType.new(subtype: subtype) }
    end

    topic = klass.create!(content: { trial: true })
    assert_equal [1] * threads.size, threads.map(&:value)
  end
end

class SerializedAttributeTestWithYamlSafeLoad < SerializedAttributeTest
  def setup
    ActiveRecord.use_yaml_unsafe_load = false
  end

  def test_serialized_attribute
    Topic.serialize("content", String)

    myobj = String.new("value1")
    topic = Topic.create("content" => myobj)
    assert_equal(myobj, topic.content)

    topic.reload
    assert_equal(myobj, topic.content)
  end

  def test_serialized_attribute_on_custom_attribute_with_default
    klass = Class.new(ActiveRecord::Base) do
      self.table_name = Topic.table_name
      attribute :content, default: { "key" => "value" }
      serialize :content, Hash
    end

    t = klass.new
    assert_equal({ "key" => "value" }, t.content)
  end

  def test_nil_is_always_persisted_as_null
    Topic.serialize(:content, Hash)

    topic = Topic.create!(content: { "foo" => "bar" })
    topic.update_attribute :content, nil
    assert_equal [topic], Topic.where(content: nil)
  end

  def test_serialized_attribute_with_default
    klass = Class.new(ActiveRecord::Base) do
      self.table_name = Topic.table_name
      serialize(:content, Hash, default: { "key" => "value" })
    end

    t = klass.new
    assert_equal({ "key" => "value" }, t.content)
  end

  def test_serialized_attributes_from_database_on_subclass
    Topic.serialize :content, Hash

    t = ImportantTopic.new(content: { "foo" => "bar" })
    assert_equal({ "foo" => "bar" }, t.content)
    t.save!
    t = ImportantTopic.last
    assert_equal({ "foo" => "bar" }, t.content)
  end

  def test_serialized_attribute_on_alias_attribute
    klass = Class.new(ActiveRecord::Base) do
      self.table_name = Topic.table_name
      alias_attribute :object, :content
      serialize :object, Hash
    end

    myobj = { "somevalue" => "thevalue" }
    topic = klass.create!(object: myobj)
    assert_equal(myobj, topic.object)

    topic.reload
    assert_equal(myobj, topic.object)
  end

  def test_unexpected_serialized_type
    Topic.serialize :content, Hash
    topic = Topic.create!(content: { "zomg" => true })

    Topic.serialize :content, Array

    topic.reload
    error = assert_raise(ActiveRecord::SerializationTypeMismatch) do
      topic.content
    end
    expected = "can't load `content`: was supposed to be a Array, but was a Hash. -- {\"zomg\"=>true}"
    assert_equal expected, error.to_s
  end

  def test_serialize_attribute_via_select_method_when_time_zone_available
    with_timezone_config aware_attributes: true do
      Topic.serialize(:content, Hash)

      myobj = { "somevalue" => "thevalue" }
      topic = Topic.create(content: myobj)

      assert_equal(myobj, Topic.select(:content).find(topic.id).content)
      assert_raise(ActiveModel::MissingAttributeError) { Topic.select(:id).find(topic.id).content }
    end
  end

  def test_should_raise_exception_on_serialized_attribute_with_type_mismatch
    myobj = { "somevalue" => "thevalue" }
    topic = Topic.new(content: myobj)
    assert topic.save
    Topic.serialize(:content, String)
    assert_raise(ActiveRecord::SerializationTypeMismatch) { Topic.find(topic.id).content }
  end

  def test_serialized_time_attribute
    skip "Time is a DisallowedClass in Psych safe_load()."
  end
end