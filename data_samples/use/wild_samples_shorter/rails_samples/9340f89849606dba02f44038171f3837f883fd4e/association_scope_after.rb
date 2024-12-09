
            conditions.each do |condition|
              if options[:through] && condition.is_a?(Hash)
                condition = disambiguate_condition(table, condition)
              end

              scope = scope.where(interpolate(condition))
            end

            conditions.each do |condition|
              condition = interpolate(condition)
              condition = disambiguate_condition(table, condition) unless i == 0

              scope = scope.where(condition)
            end
          end
        end
      end

      def disambiguate_condition(table, condition)
        if condition.is_a?(Hash)
          Hash[
            condition.map do |k, v|
              if v.is_a?(Hash)
                [k, v]
              else
                [table.table_alias || table.name, { k => v }]
              end
            end
          ]
        else
          condition
        end
      end
    end
  end
end