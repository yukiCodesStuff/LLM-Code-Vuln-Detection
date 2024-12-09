  singleton_class.attr_accessor :query_transformers
  self.query_transformers = []

  def self.eager_load!
    super
    ActiveRecord::Locking.eager_load!
    ActiveRecord::Scoping.eager_load!