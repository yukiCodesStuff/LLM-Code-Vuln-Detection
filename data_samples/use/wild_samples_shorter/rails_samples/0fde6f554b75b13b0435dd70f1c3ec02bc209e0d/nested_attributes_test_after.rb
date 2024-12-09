    assert man.reload.interests.empty?
  end

  def test_reject_if_is_not_short_circuited_if_allow_destroy_is_false
    Pirate.accepts_nested_attributes_for :ship, reject_if: ->(a) { a[:name] == "The Golden Hind" }, allow_destroy: false

    pirate = Pirate.create!(catchphrase: "Stop wastin' me time", ship_attributes: { name: "White Pearl", _destroy: "1" })
    assert_equal "White Pearl", pirate.reload.ship.name

    pirate.update!(ship_attributes: { id: pirate.ship.id, name: "The Golden Hind", _destroy: "1" })
    assert_equal "White Pearl", pirate.reload.ship.name

    pirate.update!(ship_attributes: { id: pirate.ship.id, name: "Black Pearl", _destroy: "1" })
    assert_equal "Black Pearl", pirate.reload.ship.name
  end

  def test_has_many_association_updating_a_single_record
    Man.accepts_nested_attributes_for(:interests)
    man = Man.create(name: 'John')
    interest = man.interests.create(topic: 'photography')