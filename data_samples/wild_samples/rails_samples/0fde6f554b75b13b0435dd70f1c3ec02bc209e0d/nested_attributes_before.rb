    #   assign_nested_attributes_for_collection_association(:people, {
    #     '1' => { id: '1', name: 'Peter' },
    #     '2' => { name: 'John' },
    #     '3' => { id: '2', _destroy: true }
    #   })
    #
    # Will update the name of the Person with ID 1, build a new associated
    # person with the name 'John', and mark the associated Person with ID 2
    # for destruction.
    #
    # Also accepts an Array of attribute hashes:
    #
    #   assign_nested_attributes_for_collection_association(:people, [
    #     { id: '1', name: 'Peter' },
    #     { name: 'John' },
    #     { id: '2', _destroy: true }
    #   ])
    def assign_nested_attributes_for_collection_association(association_name, attributes_collection)
      options = self.nested_attributes_options[association_name]
      if attributes_collection.respond_to?(:permitted?)
        attributes_collection = attributes_collection.to_h
      end

      unless attributes_collection.is_a?(Hash) || attributes_collection.is_a?(Array)
        raise ArgumentError, "Hash or Array expected, got #{attributes_collection.class.name} (#{attributes_collection.inspect})"
      end

      check_record_limit!(options[:limit], attributes_collection)

      if attributes_collection.is_a? Hash
        keys = attributes_collection.keys
        attributes_collection = if keys.include?('id') || keys.include?(:id)
          [attributes_collection]
        else
          attributes_collection.values
        end
      end

      association = association(association_name)

      existing_records = if association.loaded?
        association.target
      else
        attribute_ids = attributes_collection.map {|a| a['id'] || a[:id] }.compact
        attribute_ids.empty? ? [] : association.scope.where(association.klass.primary_key => attribute_ids)
      end

      attributes_collection.each do |attributes|
        if attributes.respond_to?(:permitted?)
          attributes = attributes.to_h
        end
        attributes = attributes.with_indifferent_access

        if attributes['id'].blank?
          unless reject_new_record?(association_name, attributes)
            association.build(attributes.except(*UNASSIGNABLE_KEYS))
          end
        elsif existing_record = existing_records.detect { |record| record.id.to_s == attributes['id'].to_s }
          unless call_reject_if(association_name, attributes)
            # Make sure we are operating on the actual object which is in the association's
            # proxy_target array (either by finding it, or adding it if not found)
            # Take into account that the proxy_target may have changed due to callbacks
            target_record = association.target.detect { |record| record.id.to_s == attributes['id'].to_s }
            if target_record
              existing_record = target_record
            else
              association.add_to_target(existing_record, :skip_callbacks)
            end

            assign_to_or_mark_for_destruction(existing_record, attributes, options[:allow_destroy])
          end
        else
          raise_nested_attributes_record_not_found!(association_name, attributes['id'])
        end
      end
    end

    # Takes in a limit and checks if the attributes_collection has too many
    # records. It accepts limit in the form of symbol, proc, or
    # number-like object (anything that can be compared with an integer).
    #
    # Raises TooManyRecords error if the attributes_collection is
    # larger than the limit.
    def check_record_limit!(limit, attributes_collection)
      if limit
        limit = case limit
        when Symbol
          send(limit)
        when Proc
          limit.call
        else
          limit
        end

        if limit && attributes_collection.size > limit
          raise TooManyRecords, "Maximum #{limit} records are allowed. Got #{attributes_collection.size} records instead."
        end
      end
    end

    # Updates a record with the +attributes+ or marks it for destruction if
    # +allow_destroy+ is +true+ and has_destroy_flag? returns +true+.
    def assign_to_or_mark_for_destruction(record, attributes, allow_destroy)
      record.assign_attributes(attributes.except(*UNASSIGNABLE_KEYS))
      record.mark_for_destruction if has_destroy_flag?(attributes) && allow_destroy
    end

    # Determines if a hash contains a truthy _destroy key.
    def has_destroy_flag?(hash)
      Type::Boolean.new.cast(hash['_destroy'])
    end

    # Determines if a new record should be rejected by checking
    # has_destroy_flag? or if a <tt>:reject_if</tt> proc exists for this
    # association and evaluates to +true+.
    def reject_new_record?(association_name, attributes)
      has_destroy_flag?(attributes) || call_reject_if(association_name, attributes)
    end

    # Determines if a record with the particular +attributes+ should be
    # rejected by calling the reject_if Symbol or Proc (if defined).
    # The reject_if option is defined by +accepts_nested_attributes_for+.
    #
    # Returns false if there is a +destroy_flag+ on the attributes.
    def call_reject_if(association_name, attributes)
      return false if has_destroy_flag?(attributes)
      case callback = self.nested_attributes_options[association_name][:reject_if]
      when Symbol
        method(callback).arity == 0 ? send(callback) : send(callback, attributes)
      when Proc
        callback.call(attributes)
      end
    end

    def raise_nested_attributes_record_not_found!(association_name, record_id)
      model = self.class._reflect_on_association(association_name).klass.name
      raise RecordNotFound.new("Couldn't find #{model} with ID=#{record_id} for #{self.class.name} with ID=#{id}",
                               model, 'id', record_id)
    end
  end
end
    #   assign_nested_attributes_for_collection_association(:people, {
    #     '1' => { id: '1', name: 'Peter' },
    #     '2' => { name: 'John' },
    #     '3' => { id: '2', _destroy: true }
    #   })
    #
    # Will update the name of the Person with ID 1, build a new associated
    # person with the name 'John', and mark the associated Person with ID 2
    # for destruction.
    #
    # Also accepts an Array of attribute hashes:
    #
    #   assign_nested_attributes_for_collection_association(:people, [
    #     { id: '1', name: 'Peter' },
    #     { name: 'John' },
    #     { id: '2', _destroy: true }
    #   ])
    def assign_nested_attributes_for_collection_association(association_name, attributes_collection)
      options = self.nested_attributes_options[association_name]
      if attributes_collection.respond_to?(:permitted?)
        attributes_collection = attributes_collection.to_h
      end

      unless attributes_collection.is_a?(Hash) || attributes_collection.is_a?(Array)
        raise ArgumentError, "Hash or Array expected, got #{attributes_collection.class.name} (#{attributes_collection.inspect})"
      end

      check_record_limit!(options[:limit], attributes_collection)

      if attributes_collection.is_a? Hash
        keys = attributes_collection.keys
        attributes_collection = if keys.include?('id') || keys.include?(:id)
          [attributes_collection]
        else
          attributes_collection.values
        end
      end

      association = association(association_name)

      existing_records = if association.loaded?
        association.target
      else
        attribute_ids = attributes_collection.map {|a| a['id'] || a[:id] }.compact
        attribute_ids.empty? ? [] : association.scope.where(association.klass.primary_key => attribute_ids)
      end

      attributes_collection.each do |attributes|
        if attributes.respond_to?(:permitted?)
          attributes = attributes.to_h
        end
        attributes = attributes.with_indifferent_access

        if attributes['id'].blank?
          unless reject_new_record?(association_name, attributes)
            association.build(attributes.except(*UNASSIGNABLE_KEYS))
          end
        elsif existing_record = existing_records.detect { |record| record.id.to_s == attributes['id'].to_s }
          unless call_reject_if(association_name, attributes)
            # Make sure we are operating on the actual object which is in the association's
            # proxy_target array (either by finding it, or adding it if not found)
            # Take into account that the proxy_target may have changed due to callbacks
            target_record = association.target.detect { |record| record.id.to_s == attributes['id'].to_s }
            if target_record
              existing_record = target_record
            else
              association.add_to_target(existing_record, :skip_callbacks)
            end

            assign_to_or_mark_for_destruction(existing_record, attributes, options[:allow_destroy])
          end
        else
          raise_nested_attributes_record_not_found!(association_name, attributes['id'])
        end
      end
    end

    # Takes in a limit and checks if the attributes_collection has too many
    # records. It accepts limit in the form of symbol, proc, or
    # number-like object (anything that can be compared with an integer).
    #
    # Raises TooManyRecords error if the attributes_collection is
    # larger than the limit.
    def check_record_limit!(limit, attributes_collection)
      if limit
        limit = case limit
        when Symbol
          send(limit)
        when Proc
          limit.call
        else
          limit
        end

        if limit && attributes_collection.size > limit
          raise TooManyRecords, "Maximum #{limit} records are allowed. Got #{attributes_collection.size} records instead."
        end
      end
    end

    # Updates a record with the +attributes+ or marks it for destruction if
    # +allow_destroy+ is +true+ and has_destroy_flag? returns +true+.
    def assign_to_or_mark_for_destruction(record, attributes, allow_destroy)
      record.assign_attributes(attributes.except(*UNASSIGNABLE_KEYS))
      record.mark_for_destruction if has_destroy_flag?(attributes) && allow_destroy
    end

    # Determines if a hash contains a truthy _destroy key.
    def has_destroy_flag?(hash)
      Type::Boolean.new.cast(hash['_destroy'])
    end

    # Determines if a new record should be rejected by checking
    # has_destroy_flag? or if a <tt>:reject_if</tt> proc exists for this
    # association and evaluates to +true+.
    def reject_new_record?(association_name, attributes)
      has_destroy_flag?(attributes) || call_reject_if(association_name, attributes)
    end

    # Determines if a record with the particular +attributes+ should be
    # rejected by calling the reject_if Symbol or Proc (if defined).
    # The reject_if option is defined by +accepts_nested_attributes_for+.
    #
    # Returns false if there is a +destroy_flag+ on the attributes.
    def call_reject_if(association_name, attributes)
      return false if has_destroy_flag?(attributes)
      case callback = self.nested_attributes_options[association_name][:reject_if]
      when Symbol
        method(callback).arity == 0 ? send(callback) : send(callback, attributes)
      when Proc
        callback.call(attributes)
      end
    end

    def raise_nested_attributes_record_not_found!(association_name, record_id)
      model = self.class._reflect_on_association(association_name).klass.name
      raise RecordNotFound.new("Couldn't find #{model} with ID=#{record_id} for #{self.class.name} with ID=#{id}",
                               model, 'id', record_id)
    end
  end
end
        method(callback).arity == 0 ? send(callback) : send(callback, attributes)
      when Proc
        callback.call(attributes)
      end
    end

    def raise_nested_attributes_record_not_found!(association_name, record_id)
      model = self.class._reflect_on_association(association_name).klass.name
      raise RecordNotFound.new("Couldn't find #{model} with ID=#{record_id} for #{self.class.name} with ID=#{id}",
                               model, 'id', record_id)
    end
  end
end