      included do
        # Stores the default scope for the class.
        class_attribute :default_scopes, instance_writer: false, instance_predicate: false
        class_attribute :default_scope_override, instance_predicate: false

        self.default_scopes = []
        self.default_scope_override = nil
      end