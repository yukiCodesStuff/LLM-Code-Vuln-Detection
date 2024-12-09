     PostgresqlBitString, PostgresqlOid, PostgresqlTimestampWithZone, PostgresqlUUID].each(&:delete_all)
  end

  def test_data_type_of_array_types
    assert_equal :integer, @first_array.column_for_attribute(:commission_by_quarter).type
    assert_equal :text, @first_array.column_for_attribute(:nicknames).type
  end