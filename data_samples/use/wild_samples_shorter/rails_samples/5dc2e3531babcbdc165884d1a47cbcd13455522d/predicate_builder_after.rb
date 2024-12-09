        column = reflection.foreign_key
      end

      queries << build(table[column], value)
      queries
    end

    def self.references(attributes)