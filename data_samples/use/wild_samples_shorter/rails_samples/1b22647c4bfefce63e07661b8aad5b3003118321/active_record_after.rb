  singleton_class.attr_accessor :use_yaml_unsafe_load
  self.use_yaml_unsafe_load = false

  ##
  # :singleton-method:
  # Application configurable boolean that denotes whether or not to raise
  # an exception when the PostgreSQLAdapter is provided with an integer that
  # is wider than signed 64bit representation
  singleton_class.attr_accessor :raise_int_wider_than_64bit
  self.raise_int_wider_than_64bit = true

  ##
  # :singleton-method:
  # Application configurable array that provides additional permitted classes
  # to Psych safe_load in the YAML Coder