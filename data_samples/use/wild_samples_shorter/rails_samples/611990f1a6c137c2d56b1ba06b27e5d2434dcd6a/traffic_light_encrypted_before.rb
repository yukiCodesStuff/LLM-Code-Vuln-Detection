end

class EncryptedTrafficLightWithStoreState < TrafficLight
  store :state, accessors: %i[ color ]
  encrypts :state
end