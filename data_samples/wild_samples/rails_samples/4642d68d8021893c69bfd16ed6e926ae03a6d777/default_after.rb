module ActiveRecord
  module Scoping
    module Default
      extend ActiveSupport::Concern

      included do
        # Stores the default scope for the class.
        class_attribute :default_scopes, instance_writer: false, instance_predicate: false
        class_attribute :default_scope_override, instance_writer: false, instance_predicate: false

        self.default_scopes = []
        self.default_scope_override = nil
      end

      module ClassMethods
        # Returns a scope for the model without the previously set scopes.
        #
        #   class Post < ActiveRecord::Base
        #     def self.default_scope
        #       where(published: true)
        #     end
        #   end
        #
        #   Post.all                                  # Fires "SELECT * FROM posts WHERE published = true"
        #   Post.unscoped.all                         # Fires "SELECT * FROM posts"
        #   Post.where(published: false).unscoped.all # Fires "SELECT * FROM posts"
        #
        # This method also accepts a block. All queries inside the block will
        # not use the previously set scopes.
        #
        #   Post.unscoped {
        #     Post.limit(10) # Fires "SELECT * FROM posts LIMIT 10"
        #   }