# frozen_string_literal: true

require "cases/helper"
require "models/post"
require "models/author"
require "models/topic"
require "models/reply"
require "models/category"
require "models/categorization"
require "models/company"
require "models/customer"
require "models/developer"
require "models/computer"
require "models/project"
require "models/default"
require "models/auto_id"
require "models/column_name"
require "models/subscriber"
require "models/comment"
require "models/minimalistic"
require "models/warehouse_thing"
require "models/parrot"
require "models/person"
require "models/edge"
require "models/joke"
require "models/bird"
require "models/car"
require "models/bulb"
require "models/pet"
require "models/owner"
require "concurrent/atomic/count_down_latch"
require "active_support/core_ext/enumerable"
require "active_support/core_ext/kernel/reporting"

class FirstAbstractClass < ActiveRecord::Base
  self.abstract_class = true

  connects_to database: { writing: :arunit, reading: :arunit }
end

class SecondAbstractClass < FirstAbstractClass
  self.abstract_class = true

  connects_to database: { writing: :arunit, reading: :arunit }
end

class ThirdAbstractClass < SecondAbstractClass
  self.abstract_class = true
end

class Photo < SecondAbstractClass; end
class Smarts < ActiveRecord::Base; end
class CreditCard < ActiveRecord::Base
  class PinNumber < ActiveRecord::Base
    class CvvCode < ActiveRecord::Base; end
    class SubCvvCode < CvvCode; end
  end
  class SubPinNumber < PinNumber; end
  class Brand < Category; end
end
class MasterCreditCard < ActiveRecord::Base; end
class NonExistentTable < ActiveRecord::Base; end
class TestOracleDefault < ActiveRecord::Base; end

class ReadonlyTitlePost < Post
  attr_readonly :title
end

class Weird < ActiveRecord::Base; end

class LintTest < ActiveRecord::TestCase
  include ActiveModel::Lint::Tests

  class LintModel < ActiveRecord::Base; end

  def setup
    @model = LintModel.new
  end
end

class BasicsTest < ActiveRecord::TestCase
  fixtures :topics, :companies, :developers, :projects, :computers, :accounts, :minimalistics, "warehouse-things", :authors, :author_addresses, :categorizations, :categories, :posts

  def test_generated_association_methods_module_name
    mod = Post.send(:generated_association_methods)
    assert_equal "Post::GeneratedAssociationMethods", mod.inspect
  end

  def test_generated_relation_methods_module_name
    mod = Post.send(:generated_relation_methods)
    assert_equal "Post::GeneratedRelationMethods", mod.inspect
  end

  def test_arel_attribute_normalization
    assert_equal Post.arel_table["body"], Post.arel_table[:body]
    assert_equal Post.arel_table["body"], Post.arel_table[:text]
  end

  def test_incomplete_schema_loading
    topic = Topic.first
    payload = { "foo" => 42 }
    topic.update!(content: payload)

    Topic.reset_column_information

    Topic.connection.stub(:lookup_cast_type_from_column, ->(_) { raise "Some Error" }) do
      assert_raises RuntimeError do
        Topic.columns_hash
      end
    end

    assert_equal payload, Topic.first.content
  end

  def test_column_names_are_escaped
    conn      = ActiveRecord::Base.connection
    classname = conn.class.name[/[^:]*$/]
    badchar   = {
      "SQLite3Adapter"    => '"',
      "Mysql2Adapter"     => "`",
      "PostgreSQLAdapter" => '"',
      "OracleAdapter"     => '"',
    }.fetch(classname) {
      raise "need a bad char for #{classname}"
    }