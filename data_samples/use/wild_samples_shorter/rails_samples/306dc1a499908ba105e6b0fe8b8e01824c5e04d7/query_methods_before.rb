require 'active_support/core_ext/array/wrap'

module ActiveRecord
  module QueryMethods
    extend ActiveSupport::Concern

    # WhereChain objects act as placeholder for queries in which #where does not have any parameter.
    # In this case, #where must be chained with #not to return a new relation.
    class WhereChain
      def initialize(scope)
    end

    def where!(opts, *rest) # :nodoc:
      references!(PredicateBuilder.references(opts)) if Hash === opts

      self.where_values += build_where(opts, rest)
      self
    end
    end

    def create_with!(value) # :nodoc:
      self.create_with_value = value ? create_with_value.merge(value) : {}
      self
    end

    # Specifies table from which the records will be fetched. For example: