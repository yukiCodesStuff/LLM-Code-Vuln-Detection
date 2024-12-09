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