    assert man.reload.interests.empty?
  end

  def test_has_many_association_updating_a_single_record
    Man.accepts_nested_attributes_for(:interests)
    man = Man.create(name: 'John')
    interest = man.interests.create(topic: 'photography')