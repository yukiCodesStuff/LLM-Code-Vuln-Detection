module ActiveRecord
  module Coders
    class YAMLColumnTest < ActiveRecord::TestCase
      def test_initialize_takes_class
        coder = YAMLColumn.new("attr_name", Object)
        assert_equal Object, coder.object_class
      end
        end
      end
    end
  end
end