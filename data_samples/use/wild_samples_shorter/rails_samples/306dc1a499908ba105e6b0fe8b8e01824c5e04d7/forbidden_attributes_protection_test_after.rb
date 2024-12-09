    person = Person.new
    assert_nil person.assign_attributes(ProtectedParams.new({}))
  end

  def test_create_with_checks_permitted
    params = ProtectedParams.new(first_name: 'Guille', gender: 'm')

    assert_raises(ActiveModel::ForbiddenAttributesError) do
      Person.create_with(params).create!
    end
  end

  def test_create_with_works_with_params_values
    params = ProtectedParams.new(first_name: 'Guille')

    person = Person.create_with(first_name: params[:first_name]).create!
    assert_equal 'Guille', person.first_name
  end

  def test_where_checks_permitted
    params = ProtectedParams.new(first_name: 'Guille', gender: 'm')

    assert_raises(ActiveModel::ForbiddenAttributesError) do
      Person.where(params).create!
    end
  end

  def test_where_works_with_params_values
    params = ProtectedParams.new(first_name: 'Guille')

    person = Person.where(first_name: params[:first_name]).create!
    assert_equal 'Guille', person.first_name
  end
end