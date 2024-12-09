    person = Person.new
    assert_nil person.assign_attributes(ProtectedParams.new({}))
  end
end