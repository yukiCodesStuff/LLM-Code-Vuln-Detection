      class QuotingTest < ActiveRecord::PostgreSQLTestCase
        def setup
          @conn = ActiveRecord::Base.connection
        end

        def test_type_cast_true
          assert_equal true, @conn.type_cast(true)
          value = "user posts"
          assert_equal "\"user posts\"", @conn.quote_table_name(value)
        end
      end
    end
  end
end