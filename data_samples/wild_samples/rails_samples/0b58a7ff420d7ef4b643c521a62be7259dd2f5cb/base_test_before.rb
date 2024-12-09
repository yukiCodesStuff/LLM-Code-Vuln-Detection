require "cases/helper"
require 'models/post'
require 'models/author'
require 'models/topic'
require 'models/reply'
require 'models/category'
require 'models/company'
require 'models/customer'
require 'models/developer'
require 'models/project'
require 'models/default'
require 'models/auto_id'
require 'models/boolean'
require 'models/column_name'
require 'models/subscriber'
require 'models/keyboard'
require 'models/comment'
require 'models/minimalistic'
require 'models/warehouse_thing'
require 'models/parrot'
require 'models/loose_person'
require 'models/edge'
require 'models/joke'
require 'rexml/document'
require 'active_support/core_ext/exception'

class Category < ActiveRecord::Base; end
class Categorization < ActiveRecord::Base; end
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
class Post < ActiveRecord::Base; end
class Computer < ActiveRecord::Base; end
class NonExistentTable < ActiveRecord::Base; end
class TestOracleDefault < ActiveRecord::Base; end

class ReadonlyTitlePost < Post
  attr_readonly :title
end

class Boolean < ActiveRecord::Base; end

class BasicsTest < ActiveRecord::TestCase
  fixtures :topics, :companies, :developers, :projects, :computers, :accounts, :minimalistics, 'warehouse-things', :authors, :categorizations, :categories, :posts

  def test_primary_key_with_no_id
    assert_nil Edge.primary_key
  end

  def test_select_symbol
    topic_ids = Topic.select(:id).map(&:id).sort
    assert_equal Topic.find(:all).map(&:id).sort, topic_ids
  end

  def test_table_exists
    assert !NonExistentTable.table_exists?
    assert Topic.table_exists?
  end

  def test_preserving_date_objects
    if current_adapter?(:SybaseAdapter)
      # Sybase ctlib does not (yet?) support the date type; use datetime instead.
      assert_kind_of(
        Time, Topic.find(1).last_read,
        "The last_read attribute should be of the Time class"
      )
    else
      # Oracle enhanced adapter allows to define Date attributes in model class (see topic.rb)
      assert_kind_of(
        Date, Topic.find(1).last_read,
        "The last_read attribute should be of the Date class"
      )
    end
  end

  def test_use_table_engine_for_quoting_where
    relation = Topic.where(Topic.arel_table[:id].eq(1))
    engine = relation.table.engine

    fakepool = Class.new(Struct.new(:spec)) {
      def with_connection; yield self; end
      def connection_pool; self; end
      def quote_table_name(*args); raise "lol quote_table_name"; end
    }