require "models/binary_field"

class SerializedAttributeTest < ActiveRecord::TestCase
  fixtures :topics, :posts

  MyObject = Struct.new :attribute1, :attribute2


    klass = Class.new(ActiveRecord::Base) do
      self.table_name = Topic.table_name
      store :content
      attribute :content, :encrypted, subtype: type_for_attribute(:content)
    end

    topic = klass.create!(content: { trial: true })
  def test_decorated_type_with_decorator_block
    klass = Class.new(ActiveRecord::Base) do
      self.table_name = Topic.table_name
      store :content
      attribute(:content) { |subtype| EncryptedType.new(subtype: subtype) }
    end

    topic = klass.create!(content: { trial: true })
    assert_equal [1] * threads.size, threads.map(&:value)
  end
end