end

class EncryptedTrafficLightWithStoreState < TrafficLight
  store :state, accessors: %i[ color ], coder: ActiveRecord::Coders::JSON
  encrypts :state
end