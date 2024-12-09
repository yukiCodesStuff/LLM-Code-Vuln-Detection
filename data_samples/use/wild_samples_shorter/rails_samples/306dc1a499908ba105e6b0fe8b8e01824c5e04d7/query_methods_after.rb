require 'active_support/core_ext/array/wrap'
require 'active_model/forbidden_attributes_protection'

module ActiveRecord
  module QueryMethods
    extend ActiveSupport::Concern

    include ActiveModel::ForbiddenAttributesProtection

    # WhereChain objects act as placeholder for queries in which #where does not have any parameter.
    # In this case, #where must be chained with #not to return a new relation.
    class WhereChain
      def initialize(scope)
    end

    def where!(opts, *rest) # :nodoc:
      if Hash === opts
        opts = sanitize_forbidden_attributes(opts)
        references!(PredicateBuilder.references(opts))
      end

      self.where_values += build_where(opts, rest)
      self
    end
    end

    def create_with!(value) # :nodoc:
      if value
        value = sanitize_forbidden_attributes(value)
        self.create_with_value = create_with_value.merge(value)
      else
        self.create_with_value = {}
      end

      self
    end

    # Specifies table from which the records will be fetched. For example: