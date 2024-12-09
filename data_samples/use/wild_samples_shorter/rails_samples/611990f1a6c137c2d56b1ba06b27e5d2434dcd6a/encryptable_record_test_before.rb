  end

  test "encrypts serialized attributes" do
    states = %i[ green red ]
    traffic_light = EncryptedTrafficLight.create!(state: states, long_state: states)
    assert_encrypted_attribute(traffic_light, :state, states)
  end

  test "encrypts store attributes with accessors" do
    traffic_light = EncryptedTrafficLightWithStoreState.create!(color: "red", long_state: %i[ green red ])
    assert_equal "red", traffic_light.color
    assert_encrypted_attribute(traffic_light, :state, { "color" => "red" })
  end
