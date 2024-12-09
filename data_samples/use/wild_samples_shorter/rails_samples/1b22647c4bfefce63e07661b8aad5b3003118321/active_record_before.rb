  singleton_class.attr_accessor :use_yaml_unsafe_load
  self.use_yaml_unsafe_load = false

  ##
  # :singleton-method:
  # Application configurable array that provides additional permitted classes
  # to Psych safe_load in the YAML Coder