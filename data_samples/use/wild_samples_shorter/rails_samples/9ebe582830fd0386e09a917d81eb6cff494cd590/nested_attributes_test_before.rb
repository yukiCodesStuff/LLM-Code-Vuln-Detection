    assert_equal 'Davy Jones Gold Dagger', @pirate.ship.name
  end

  def test_should_take_a_hash_with_string_keys_and_update_the_associated_model
    @pirate.reload.ship_attributes = { 'id' => @ship.id, 'name' => 'Davy Jones Gold Dagger' }

    assert_equal @ship, @pirate.ship
    assert_equal 'Arr', @ship.pirate.catchphrase
  end

  def test_should_associate_with_record_if_parent_record_is_not_saved
    @ship.destroy
    @pirate = Pirate.create(:catchphrase => 'Arr')
    @ship = Ship.new(:name => 'Nights Dirty Lightning', :pirate_attributes => { :id => @pirate.id, :catchphrase => @pirate.catchphrase})

    assert_equal @ship.name, 'Nights Dirty Lightning'
    assert_equal @pirate, @ship.pirate
  end

  def test_should_take_a_hash_with_string_keys_and_update_the_associated_model
    @ship.reload.pirate_attributes = { 'id' => @pirate.id, 'catchphrase' => 'Arr' }
    assert_equal ['Grace OMalley', 'Privateers Greed'], [@child_1.reload.name, @child_2.reload.name]
  end

  def test_should_assign_existing_children_if_parent_is_new
    @pirate = Pirate.new({:catchphrase => "Don' botharr talkin' like one, savvy?"}.merge(@alternate_params))
    assert_equal ['Grace OMalley', 'Privateers Greed'], [@pirate.send(@association_name)[0].name, @pirate.send(@association_name)[1].name]
  end

  def test_should_also_work_with_a_HashWithIndifferentAccess
    @pirate.send(association_setter, HashWithIndifferentAccess.new('foo' => HashWithIndifferentAccess.new(:id => @child_1.id, :name => 'Grace OMalley')))
    @pirate.save
    assert_equal 'Grace OMalley', @child_1.reload.name
    assert_equal ['Grace OMalley', 'Privateers Greed'], [@child_1.name, @child_2.name]
  end

  def test_should_not_raise_RecordNotFound_if_an_id_is_given_but_doesnt_return_a_record
    assert_nothing_raised ActiveRecord::RecordNotFound do
      @pirate.attributes = { association_getter => [{ :id => 1234567890 }] }
    end
  end
