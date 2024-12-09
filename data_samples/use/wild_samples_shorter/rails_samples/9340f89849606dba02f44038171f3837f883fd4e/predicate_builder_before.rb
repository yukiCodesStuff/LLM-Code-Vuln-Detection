
        if value.is_a?(Hash)
          table = Arel::Table.new(column, engine)
          build_from_hash(engine, value, table)
        else
          column = column.to_s

          if column.include?('.')