    extend ActiveSupport::Concern

    included do
      class_attribute :_reflections, instance_writer: false
      class_attribute :aggregate_reflections, instance_writer: false
      self._reflections = {}
      self.aggregate_reflections = {}
    end
