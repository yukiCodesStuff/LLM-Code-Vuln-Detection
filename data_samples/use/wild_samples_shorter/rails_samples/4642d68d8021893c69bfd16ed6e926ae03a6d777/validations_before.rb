      include HelperMethods

      attr_accessor :validation_context
      define_callbacks :validate, scope: :name

      class_attribute :_validators
      self._validators = Hash.new { |h,k| h[k] = [] }
    end

    module ClassMethods