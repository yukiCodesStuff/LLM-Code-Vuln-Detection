        def test_quote_range
          range = "1,2]'; SELECT * FROM users; --".."a"
          c = PostgreSQLColumn.new(nil, nil, OID::Range.new(Type::Integer.new, :int8range))
          assert_equal "[1,2]''; SELECT * FROM users; --,a]::int8range", @conn.quote(range, c)
        end

        def test_quote_bit_string
          c = PostgreSQLColumn.new(nil, 1, OID::Bit.new)