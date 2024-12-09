        column = reflection.foreign_key
      end

      queries << build(table[column.to_sym], value)
      queries
    end

    def self.references(attributes)