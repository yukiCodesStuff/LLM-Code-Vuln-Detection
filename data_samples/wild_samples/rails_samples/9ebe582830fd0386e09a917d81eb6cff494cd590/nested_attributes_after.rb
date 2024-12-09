    #   params = { :member => {
    #     :posts_attributes => [{ :id => '2', :_destroy => '1' }]
    #   }}
    #
    #   member.attributes = params['member']
    #   member.posts.detect { |p| p.id == 2 }.marked_for_destruction? # => true
    #   member.posts.length # => 2
    #   member.save
    #   member.reload.posts.length # => 1
    #
    # === Saving
    #
    # All changes to models, including the destruction of those marked for
    # destruction, are saved and destroyed automatically and atomically when
    # the parent model is saved. This happens inside the transaction initiated
    # by the parents save method. See ActiveRecord::AutosaveAssociation.
    #
    # === Using with attr_accessible
    #
    # The use of <tt>attr_accessible</tt> can interfere with nested attributes
    # if you're not careful. For example, if the <tt>Member</tt> model above
    # was using <tt>attr_accessible</tt> like this:
    #
    #   attr_accessible :name
    #
    # You would need to modify it to look like this:
    #
    #   attr_accessible :name, :posts_attributes
    #
    # === Validating the presence of a parent model
    #
    # If you want to validate that a child record is associated with a parent
    # record, you can use <tt>validates_presence_of</tt> and
    # <tt>inverse_of</tt> as this example illustrates:
    #
    #   class Member < ActiveRecord::Base
    #     has_many :posts, :inverse_of => :member
    #     accepts_nested_attributes_for :posts
    #   end
    #
    #   class Post < ActiveRecord::Base
    #     belongs_to :member, :inverse_of => :posts
    #     validates_presence_of :member
    #   end
    module ClassMethods
      REJECT_ALL_BLANK_PROC = proc { |attributes| attributes.all? { |_, value| value.blank? } }

      # Defines an attributes writer for the specified association(s). If you
      # are using <tt>attr_protected</tt> or <tt>attr_accessible</tt>, then you
      # will need to add the attribute writer to the allowed list.
      #
      # Supported options:
      # [:allow_destroy]
      #   If true, destroys any members from the attributes hash with a
      #   <tt>_destroy</tt> key and a value that evaluates to +true+
      #   (eg. 1, '1', true, or 'true'). This option is off by default.
      # [:reject_if]
      #   Allows you to specify a Proc or a Symbol pointing to a method
      #   that checks whether a record should be built for a certain attribute
      #   hash. The hash is passed to the supplied Proc or the method
      #   and it should return either +true+ or +false+. When no :reject_if
      #   is specified, a record will be built for all attribute hashes that
      #   do not have a <tt>_destroy</tt> value that evaluates to true.
      #   Passing <tt>:all_blank</tt> instead of a Proc will create a proc
      #   that will reject a record where all the attributes are blank.
      # [:limit]
      #   Allows you to specify the maximum number of the associated records that
      #   can be processed with the nested attributes. If the size of the
      #   nested attributes array exceeds the specified limit, NestedAttributes::TooManyRecords
      #   exception is raised. If omitted, any number associations can be processed.
      #   Note that the :limit option is only applicable to one-to-many associations.
      # [:update_only]
      #   Allows you to specify that an existing record may only be updated.
      #   A new record may only be created when there is no existing record.
      #   This option only works for one-to-one associations and is ignored for
      #   collection associations. This option is off by default.
      #
      # Examples:
      #   # creates avatar_attributes=
      #   accepts_nested_attributes_for :avatar, :reject_if => proc { |attributes| attributes['name'].blank? }
      #   # creates avatar_attributes=
      #   accepts_nested_attributes_for :avatar, :reject_if => :all_blank
      #   # creates avatar_attributes= and posts_attributes=
      #   accepts_nested_attributes_for :avatar, :posts, :allow_destroy => true
      def accepts_nested_attributes_for(*attr_names)
        options = { :allow_destroy => false, :update_only => false }
        options.update(attr_names.extract_options!)
        options.assert_valid_keys(:allow_destroy, :reject_if, :limit, :update_only)
        options[:reject_if] = REJECT_ALL_BLANK_PROC if options[:reject_if] == :all_blank

        attr_names.each do |association_name|
          if reflection = reflect_on_association(association_name)
            reflection.options[:autosave] = true
            add_autosave_association_callbacks(reflection)
            nested_attributes_options[association_name.to_sym] = options
            type = (reflection.collection? ? :collection : :one_to_one)

            # def pirate_attributes=(attributes)
            #   assign_nested_attributes_for_one_to_one_association(:pirate, attributes)
            # end
            class_eval <<-eoruby, __FILE__, __LINE__ + 1
              if method_defined?(:#{association_name}_attributes=)
                remove_method(:#{association_name}_attributes=)
              end
              def #{association_name}_attributes=(attributes)
                assign_nested_attributes_for_#{type}_association(:#{association_name}, attributes)
              end
            eoruby
          else
            raise ArgumentError, "No association found for name `#{association_name}'. Has it been defined yet?"
          end
        end
      end
    end

    # Returns ActiveRecord::AutosaveAssociation::marked_for_destruction? It's
    # used in conjunction with fields_for to build a form element for the
    # destruction of this association.
    #
    # See ActionView::Helpers::FormHelper::fields_for for more info.
    def _destroy
      marked_for_destruction?
    end

    private

    # Attribute hash keys that should not be assigned as normal attributes.
    # These hash keys are nested attributes implementation details.
    UNASSIGNABLE_KEYS = %w( id _destroy )

    # Assigns the given attributes to the association.
    #
    # If update_only is false and the given attributes include an <tt>:id</tt>
    # that matches the existing record's id, then the existing record will be
    # modified. If update_only is true, a new record is only created when no
    # object exists. Otherwise a new record will be built.
    #
    # If the given attributes include a matching <tt>:id</tt> attribute, or
    # update_only is true, and a <tt>:_destroy</tt> key set to a truthy value,
    # then the existing record will be marked for destruction.
    def assign_nested_attributes_for_one_to_one_association(association_name, attributes)
      options = nested_attributes_options[association_name]
      attributes = attributes.with_indifferent_access
      check_existing_record = (options[:update_only] || !attributes['id'].blank?)

      if check_existing_record && (record = send(association_name)) &&
          (options[:update_only] || record.id.to_s == attributes['id'].to_s)
        assign_to_or_mark_for_destruction(record, attributes, options[:allow_destroy]) unless call_reject_if(association_name, attributes)

      elsif attributes['id']
        raise_nested_attributes_record_not_found(association_name, attributes['id'])

      elsif !reject_new_record?(association_name, attributes)
        method = "build_#{association_name}"
        if respond_to?(method)
          send(method, attributes.except(*UNASSIGNABLE_KEYS))
        else
          raise ArgumentError, "Cannot build association #{association_name}. Are you trying to build a polymorphic one-to-one association?"
        end
      end
    end

    # Assigns the given attributes to the collection association.
    #
    # Hashes with an <tt>:id</tt> value matching an existing associated record
    # will update that record. Hashes without an <tt>:id</tt> value will build
    # a new record for the association. Hashes with a matching <tt>:id</tt>
    # value and a <tt>:_destroy</tt> key set to a truthy value will mark the
    # matched record for destruction.
    #
    # For example:
    #
    #   assign_nested_attributes_for_collection_association(:people, {
    #     '1' => { :id => '1', :name => 'Peter' },
    #     '2' => { :name => 'John' },
    #     '3' => { :id => '2', :_destroy => true }
    #   assign_nested_attributes_for_collection_association(:people, {
    #     '1' => { :id => '1', :name => 'Peter' },
    #     '2' => { :name => 'John' },
    #     '3' => { :id => '2', :_destroy => true }
    #   })
    #
    # Will update the name of the Person with ID 1, build a new associated
    # person with the name `John', and mark the associated Person with ID 2
    # for destruction.
    #
    # Also accepts an Array of attribute hashes:
    #
    #   assign_nested_attributes_for_collection_association(:people, [
    #     { :id => '1', :name => 'Peter' },
    #     { :name => 'John' },
    #     { :id => '2', :_destroy => true }
    #   ])
    def assign_nested_attributes_for_collection_association(association_name, attributes_collection)
      options = nested_attributes_options[association_name]

      unless attributes_collection.is_a?(Hash) || attributes_collection.is_a?(Array)
        raise ArgumentError, "Hash or Array expected, got #{attributes_collection.class.name} (#{attributes_collection.inspect})"
      end

      if options[:limit] && attributes_collection.size > options[:limit]
        raise TooManyRecords, "Maximum #{options[:limit]} records are allowed. Got #{attributes_collection.size} records instead."
      end

      if attributes_collection.is_a? Hash
        keys = attributes_collection.keys
        attributes_collection = if keys.include?('id') || keys.include?(:id)
          Array.wrap(attributes_collection)
        else
          attributes_collection.sort_by { |i, _| i.to_i }.map { |_, attributes| attributes }
        end
      end

      association = send(association_name)

      existing_records = if association.loaded?
        association.to_a
      else
        attribute_ids = attributes_collection.map {|a| a['id'] || a[:id] }.compact
        attribute_ids.empty? ? [] : association.all(:conditions => {association.primary_key => attribute_ids})
      end

      attributes_collection.each do |attributes|
        attributes = attributes.with_indifferent_access

        if attributes['id'].blank?
          unless reject_new_record?(association_name, attributes)
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

    # Updates a record with the +attributes+ or marks it for destruction if
    # +allow_destroy+ is +true+ and has_destroy_flag? returns +true+.
    def assign_to_or_mark_for_destruction(record, attributes, allow_destroy)
      if has_destroy_flag?(attributes) && allow_destroy
        record.mark_for_destruction
      else
        record.attributes = attributes.except(*UNASSIGNABLE_KEYS)
      end
    end

    # Determines if a hash contains a truthy _destroy key.
    def has_destroy_flag?(hash)
      ConnectionAdapters::Column.value_to_boolean(hash['_destroy'])
    end

    # Determines if a new record should be build by checking for
    # has_destroy_flag? or if a <tt>:reject_if</tt> proc exists for this
    # association and evaluates to +true+.
    def reject_new_record?(association_name, attributes)
      has_destroy_flag?(attributes) || call_reject_if(association_name, attributes)
    end

    def call_reject_if(association_name, attributes)
      case callback = nested_attributes_options[association_name][:reject_if]
      when Symbol
        method(callback).arity == 0 ? send(callback) : send(callback, attributes)
      when Proc
        callback.call(attributes)
      end
    end

    def raise_nested_attributes_record_not_found(association_name, record_id)
      reflection = self.class.reflect_on_association(association_name)
      raise RecordNotFound, "Couldn't find #{reflection.klass.name} with ID=#{record_id} for #{self.class.name} with ID=#{id}"
    end
  end
end
    #   assign_nested_attributes_for_collection_association(:people, {
    #     '1' => { :id => '1', :name => 'Peter' },
    #     '2' => { :name => 'John' },
    #     '3' => { :id => '2', :_destroy => true }
    #   })
    #
    # Will update the name of the Person with ID 1, build a new associated
    # person with the name `John', and mark the associated Person with ID 2
    # for destruction.
    #
    # Also accepts an Array of attribute hashes:
    #
    #   assign_nested_attributes_for_collection_association(:people, [
    #     { :id => '1', :name => 'Peter' },
    #     { :name => 'John' },
    #     { :id => '2', :_destroy => true }
    #   ])
    def assign_nested_attributes_for_collection_association(association_name, attributes_collection)
      options = nested_attributes_options[association_name]

      unless attributes_collection.is_a?(Hash) || attributes_collection.is_a?(Array)
        raise ArgumentError, "Hash or Array expected, got #{attributes_collection.class.name} (#{attributes_collection.inspect})"
      end

      if options[:limit] && attributes_collection.size > options[:limit]
        raise TooManyRecords, "Maximum #{options[:limit]} records are allowed. Got #{attributes_collection.size} records instead."
      end

      if attributes_collection.is_a? Hash
        keys = attributes_collection.keys
        attributes_collection = if keys.include?('id') || keys.include?(:id)
          Array.wrap(attributes_collection)
        else
          attributes_collection.sort_by { |i, _| i.to_i }.map { |_, attributes| attributes }
        end
      end

      association = send(association_name)

      existing_records = if association.loaded?
        association.to_a
      else
        attribute_ids = attributes_collection.map {|a| a['id'] || a[:id] }.compact
        attribute_ids.empty? ? [] : association.all(:conditions => {association.primary_key => attribute_ids})
      end

      attributes_collection.each do |attributes|
        attributes = attributes.with_indifferent_access

        if attributes['id'].blank?
          unless reject_new_record?(association_name, attributes)
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

    # Updates a record with the +attributes+ or marks it for destruction if
    # +allow_destroy+ is +true+ and has_destroy_flag? returns +true+.
    def assign_to_or_mark_for_destruction(record, attributes, allow_destroy)
      if has_destroy_flag?(attributes) && allow_destroy
        record.mark_for_destruction
      else
        record.attributes = attributes.except(*UNASSIGNABLE_KEYS)
      end
    end

    # Determines if a hash contains a truthy _destroy key.
    def has_destroy_flag?(hash)
      ConnectionAdapters::Column.value_to_boolean(hash['_destroy'])
    end

    # Determines if a new record should be build by checking for
    # has_destroy_flag? or if a <tt>:reject_if</tt> proc exists for this
    # association and evaluates to +true+.
    def reject_new_record?(association_name, attributes)
      has_destroy_flag?(attributes) || call_reject_if(association_name, attributes)
    end

    def call_reject_if(association_name, attributes)
      case callback = nested_attributes_options[association_name][:reject_if]
      when Symbol
        method(callback).arity == 0 ? send(callback) : send(callback, attributes)
      when Proc
        callback.call(attributes)
      end
    end

    def raise_nested_attributes_record_not_found(association_name, record_id)
      reflection = self.class.reflect_on_association(association_name)
      raise RecordNotFound, "Couldn't find #{reflection.klass.name} with ID=#{record_id} for #{self.class.name} with ID=#{id}"
    end
  end
end
        method(callback).arity == 0 ? send(callback) : send(callback, attributes)
      when Proc
        callback.call(attributes)
      end
    end

    def raise_nested_attributes_record_not_found(association_name, record_id)
      reflection = self.class.reflect_on_association(association_name)
      raise RecordNotFound, "Couldn't find #{reflection.klass.name} with ID=#{record_id} for #{self.class.name} with ID=#{id}"
    end
  end
end