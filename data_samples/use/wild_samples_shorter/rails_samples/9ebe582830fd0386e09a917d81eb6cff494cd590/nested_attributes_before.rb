        assign_to_or_mark_for_destruction(record, attributes, options[:allow_destroy]) unless call_reject_if(association_name, attributes)

      elsif attributes['id']
        existing_record = self.class.reflect_on_association(association_name).klass.find(attributes['id'])
        assign_to_or_mark_for_destruction(existing_record, attributes, options[:allow_destroy])
        self.send(association_name.to_s+'=', existing_record)

      elsif !reject_new_record?(association_name, attributes)
        method = "build_#{association_name}"
        if respond_to?(method)
            association.build(attributes.except(*UNASSIGNABLE_KEYS))
          end

        elsif existing_records.count == 0 #Existing record but not yet associated
          existing_record = self.class.reflect_on_association(association_name).klass.find(attributes['id'])
          association.send(:add_record_to_target_with_callbacks, existing_record) if !association.loaded? && !call_reject_if(association_name, attributes)
          assign_to_or_mark_for_destruction(existing_record, attributes, options[:allow_destroy])

        elsif existing_record = existing_records.detect { |record| record.id.to_s == attributes['id'].to_s }
          association.send(:add_record_to_target_with_callbacks, existing_record) if !association.loaded? && !call_reject_if(association_name, attributes)
          assign_to_or_mark_for_destruction(existing_record, attributes, options[:allow_destroy])

        end
      end
    end

      ConnectionAdapters::Column.value_to_boolean(hash['_destroy'])
    end

    # Determines if a new record should be built by checking for
    # has_destroy_flag? or if a <tt>:reject_if</tt> proc exists for this
    # association and evaluates to +true+.
    def reject_new_record?(association_name, attributes)
      has_destroy_flag?(attributes) || call_reject_if(association_name, attributes)
      end
    end

  end
end