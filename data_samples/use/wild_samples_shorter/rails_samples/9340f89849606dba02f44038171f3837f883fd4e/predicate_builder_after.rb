
        if value.is_a?(Hash)
          table = Arel::Table.new(column, engine)
          value.map { |k,v| build(table[k.to_sym], v) }
        else
          column = column.to_s

          if column.include?('.')