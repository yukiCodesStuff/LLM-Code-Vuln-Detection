
  module Enum
    def self.extended(base) # :nodoc:
      base.class_attribute(:defined_enums)
      base.defined_enums = {}
    end

    def inherited(base) # :nodoc: