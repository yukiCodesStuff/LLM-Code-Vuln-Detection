    extend ActiveSupport::Concern

    included do
      class_attribute :_reflections
      class_attribute :aggregate_reflections
      self._reflections = {}
      self.aggregate_reflections = {}
    end
