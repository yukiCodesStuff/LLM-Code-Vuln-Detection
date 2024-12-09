    assert_equal 'Davy Jones Gold Dagger', @pirate.ship.name
  end

  def test_should_raise_RecordNotFound_if_an_id_is_given_but_doesnt_return_a_record
    assert_raise_with_message ActiveRecord::RecordNotFound, "Couldn't find Ship with ID=1234567890 for Pirate with ID=#{@pirate.id}" do
      @pirate.ship_attributes = { :id => 1234567890 }
    end
  end

  def test_should_take_a_hash_with_string_keys_and_update_the_associated_model
    @pirate.reload.ship_attributes = { 'id' => @ship.id, 'name' => 'Davy Jones Gold Dagger' }

    assert_equal @ship, @pirate.ship
    assert_equal 'Arr', @ship.pirate.catchphrase
  end

  def test_should_raise_RecordNotFound_if_an_id_is_given_but_doesnt_return_a_record
    assert_raise_with_message ActiveRecord::RecordNotFound, "Couldn't find Pirate with ID=1234567890 for Ship with ID=#{@ship.id}" do
      @ship.pirate_attributes = { :id => 1234567890 }
    end
  end

  def test_should_take_a_hash_with_string_keys_and_update_the_associated_model
    @ship.reload.pirate_attributes = { 'id' => @pirate.id, 'catchphrase' => 'Arr' }
    assert_equal ['Grace OMalley', 'Privateers Greed'], [@child_1.reload.name, @child_2.reload.name]
  end

  def test_should_also_work_with_a_HashWithIndifferentAccess
    @pirate.send(association_setter, HashWithIndifferentAccess.new('foo' => HashWithIndifferentAccess.new(:id => @child_1.id, :name => 'Grace OMalley')))
    @pirate.save
    assert_equal 'Grace OMalley', @child_1.reload.name
    assert_equal ['Grace OMalley', 'Privateers Greed'], [@child_1.name, @child_2.name]
  end

  def test_should_raise_RecordNotFound_if_an_id_is_given_but_doesnt_return_a_record
    assert_raise_with_message ActiveRecord::RecordNotFound, "Couldn't find #{@child_1.class.name} with ID=1234567890 for Pirate with ID=#{@pirate.id}" do
      @pirate.attributes = { association_getter => [{ :id => 1234567890 }] }
    end
  end
