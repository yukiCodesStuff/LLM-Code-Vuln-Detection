      include HelperMethods

      attr_accessor :validation_context
      private :validation_context=
      define_callbacks :validate, scope: :name

      class_attribute :_validators, instance_writer: false
      self._validators = Hash.new { |h,k| h[k] = [] }
    end

    module ClassMethods