    assert_equal(-2.25, type.cast(+"(2.25)"))
  end

  def test_sum_with_type_cast
    @connection.execute("INSERT INTO postgresql_moneys (id, wealth) VALUES (1, '123.45'::money)")

    assert_equal BigDecimal("123.45"), PostgresqlMoney.sum("id * wealth")