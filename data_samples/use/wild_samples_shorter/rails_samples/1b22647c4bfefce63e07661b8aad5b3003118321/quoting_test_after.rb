      class QuotingTest < ActiveRecord::PostgreSQLTestCase
        def setup
          @conn = ActiveRecord::Base.connection
          @raise_int_wider_than_64bit = ActiveRecord.raise_int_wider_than_64bit
        end

        def test_type_cast_true
          assert_equal true, @conn.type_cast(true)
          value = "user posts"
          assert_equal "\"user posts\"", @conn.quote_table_name(value)
        end

        def test_raise_when_int_is_wider_than_64bit
          value = 9223372036854775807 + 1
          assert_raise ActiveRecord::ConnectionAdapters::PostgreSQL::Quoting::IntegerOutOf64BitRange do
            @conn.quote(value)
          end

          value = -9223372036854775808 - 1
          assert_raise ActiveRecord::ConnectionAdapters::PostgreSQL::Quoting::IntegerOutOf64BitRange do
            @conn.quote(value)
          end
        end

        def test_do_not_raise_when_int_is_not_wider_than_64bit
          value = 9223372036854775807
          assert_equal "9223372036854775807", @conn.quote(value)

          value = -9223372036854775808
          assert_equal "-9223372036854775808", @conn.quote(value)
        end

        def test_do_not_raise_when_raise_int_wider_than_64bit_is_false
          ActiveRecord.raise_int_wider_than_64bit = false
          value = 9223372036854775807 + 1
          assert_equal "9223372036854775808", @conn.quote(value)
          ActiveRecord.raise_int_wider_than_64bit = @raise_int_wider_than_64bit
        end
      end
    end
  end
end