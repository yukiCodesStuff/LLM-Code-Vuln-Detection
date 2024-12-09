
            conditions.each do |condition|
              if options[:through] && condition.is_a?(Hash)
                condition = { table.name => condition }
              end

              scope = scope.where(interpolate(condition))
            end

            conditions.each do |condition|
              condition = interpolate(condition)
              condition = { (table.table_alias || table.name) => condition } unless i == 0

              scope = scope.where(condition)
            end
          end
        end
      end

    end
  end
end