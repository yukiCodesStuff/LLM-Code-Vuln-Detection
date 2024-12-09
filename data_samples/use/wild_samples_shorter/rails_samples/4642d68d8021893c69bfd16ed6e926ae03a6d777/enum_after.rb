
  module Enum
    def self.extended(base) # :nodoc:
      base.class_attribute(:defined_enums, instance_writer: false)
      base.defined_enums = {}
    end

    def inherited(base) # :nodoc: