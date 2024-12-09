        assert_deprecated {
          range = PostgresqlRange.create!(int8_range: "(10, 100]")
          assert_equal 11..100, range.int8_range
        }
      }
    end

    def test_exclude_beginning_for_subtypes_without_succ_method_is_not_supported
      assert_raises(ArgumentError) { PostgresqlRange.create!(num_range: "(0.1, 0.2]") }
      assert_raises(ArgumentError) { PostgresqlRange.create!(float_range: "(0.5, 0.7]") }
    end

    def test_update_all_with_ranges
      PostgresqlRange.create!

      PostgresqlRange.update_all(int8_range: 1..100)

      assert_equal 1...101, PostgresqlRange.first.int8_range
    end

    def test_ranges_correctly_escape_input
      e = assert_raises(ActiveRecord::StatementInvalid) do
        range = "1,2]'; SELECT * FROM users; --".."a"
        PostgresqlRange.update_all(int8_range: range)
      end

      assert e.message.starts_with?("PG::InvalidTextRepresentation")
    end

    private
      def assert_equal_round_trip(range, attribute, value)
        round_trip(range, attribute, value)
        assert_equal value, range.public_send(attribute)
      end

      def assert_nil_round_trip(range, attribute, value)
        round_trip(range, attribute, value)
        assert_nil range.public_send(attribute)
      end

      def round_trip(range, attribute, value)
        range.public_send "#{attribute}=", value
        assert range.save
        assert range.reload
      end

      def insert_range(values)
        @connection.execute <<-SQL
          INSERT INTO postgresql_ranges (
            id,
            date_range,
            num_range,
            ts_range,
            tstz_range,
            int4_range,
            int8_range,
            float_range
          ) VALUES (
            #{values[:id]},
            '#{values[:date_range]}',
            '#{values[:num_range]}',
            '#{values[:ts_range]}',
            '#{values[:tstz_range]}',
            '#{values[:int4_range]}',
            '#{values[:int8_range]}',
            '#{values[:float_range]}'
          )
        SQL
      end
  end
end