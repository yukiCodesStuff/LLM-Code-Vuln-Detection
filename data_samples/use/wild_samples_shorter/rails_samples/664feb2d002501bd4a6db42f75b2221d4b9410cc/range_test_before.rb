      assert_raises(ArgumentError) { PostgresqlRange.create!(float_range: "(0.5, 0.7]") }
    end

    private
      def assert_equal_round_trip(range, attribute, value)
        round_trip(range, attribute, value)
        assert_equal value, range.public_send(attribute)