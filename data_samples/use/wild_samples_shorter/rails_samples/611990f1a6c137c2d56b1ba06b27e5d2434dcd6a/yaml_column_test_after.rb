module ActiveRecord
  module Coders
    class YAMLColumnTest < ActiveRecord::TestCase
      setup do
        ActiveRecord.use_yaml_unsafe_load = true
      end

      def test_initialize_takes_class
        coder = YAMLColumn.new("attr_name", Object)
        assert_equal Object, coder.object_class
      end
        end
      end
    end

    class YAMLColumnTestWithSafeLoad < YAMLColumnTest
      setup do
        @yaml_column_permitted_classes_default = ActiveRecord.yaml_column_permitted_classes
        ActiveRecord.use_yaml_unsafe_load = false
      end

      def test_yaml_column_permitted_classes_are_consumed_by_safe_load
        ActiveRecord.yaml_column_permitted_classes = [Symbol, Time]

        coder = YAMLColumn.new("attr_name")
        time_yaml = YAML.dump(Time.new)
        symbol_yaml = YAML.dump(:somesymbol)

        assert_nothing_raised do
          coder.load(time_yaml)
          coder.load(symbol_yaml)
        end

        ActiveRecord.yaml_column_permitted_classes = @yaml_column_permitted_classes_default
      end

      def test_load_doesnt_handle_undefined_class_or_module
        coder = YAMLColumn.new("attr_name")
        missing_class_yaml = '--- !ruby/object:DoesNotExistAndShouldntEver {}\n'
        assert_raises(Psych::DisallowedClass) do
          coder.load(missing_class_yaml)
        end
      end
    end
  end
end