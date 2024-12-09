  singleton_class.attr_accessor :query_transformers
  self.query_transformers = []

  ##
  # :singleton-method:
  # Application configurable boolean that instructs the YAML Coder to use
  # an unsafe load if set to true.
  singleton_class.attr_accessor :use_yaml_unsafe_load
  self.use_yaml_unsafe_load = false

  ##
  # :singleton-method:
  # Application configurable array that provides additional permitted classes
  # to Psych safe_load in the YAML Coder
  singleton_class.attr_accessor :yaml_column_permitted_classes
  self.yaml_column_permitted_classes = []

  def self.eager_load!
    super
    ActiveRecord::Locking.eager_load!
    ActiveRecord::Scoping.eager_load!