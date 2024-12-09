    assert_equal(-2.25, type.cast(+"(2.25)"))
  end

  def test_money_regex_backtracking
    type = PostgresqlMoney.type_for_attribute("wealth")
    Timeout.timeout(0.1) do
      assert_equal(0.0, type.cast("$" + "," * 100000 + ".11!"))
      assert_equal(0.0, type.cast("$" + "." * 100000 + ",11!"))
    end
  end

  def test_sum_with_type_cast
    @connection.execute("INSERT INTO postgresql_moneys (id, wealth) VALUES (1, '123.45'::money)")

    assert_equal BigDecimal("123.45"), PostgresqlMoney.sum("id * wealth")