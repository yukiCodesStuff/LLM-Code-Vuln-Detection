        assign_to_or_mark_for_destruction(record, attributes, options[:allow_destroy]) unless call_reject_if(association_name, attributes)

      elsif attributes['id']
        raise_nested_attributes_record_not_found(association_name, attributes['id'])

      elsif !reject_new_record?(association_name, attributes)
        method = "build_#{association_name}"
        if respond_to?(method)
            association.build(attributes.except(*UNASSIGNABLE_KEYS))
          end

        elsif existing_record = existing_records.detect { |record| record.id.to_s == attributes['id'].to_s }
          association.send(:add_record_to_target_with_callbacks, existing_record) if !association.loaded? && !call_reject_if(association_name, attributes)
          assign_to_or_mark_for_destruction(existing_record, attributes, options[:allow_destroy])

        else
          raise_nested_attributes_record_not_found(association_name, attributes['id'])
        end
      end
    end

      ConnectionAdapters::Column.value_to_boolean(hash['_destroy'])
    end

    # Determines if a new record should be build by checking for
    # has_destroy_flag? or if a <tt>:reject_if</tt> proc exists for this
    # association and evaluates to +true+.
    def reject_new_record?(association_name, attributes)
      has_destroy_flag?(attributes) || call_reject_if(association_name, attributes)
      end
    end

    def raise_nested_attributes_record_not_found(association_name, record_id)
      reflection = self.class.reflect_on_association(association_name)
      raise RecordNotFound, "Couldn't find #{reflection.klass.name} with ID=#{record_id} for #{self.class.name} with ID=#{id}"
    end
  end
end