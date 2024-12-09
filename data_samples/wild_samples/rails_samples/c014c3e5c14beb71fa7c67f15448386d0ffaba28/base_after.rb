            respond_to?(:"#{k}=") ? send(:"#{k}=", v) : raise(UnknownAttributeError, "unknown attribute: #{k}")
          end
        end

        assign_multiparameter_attributes(multi_parameter_attributes)
      end

      # Returns a hash of all the attributes with their names as keys and the values of the attributes as values.
      def attributes
        self.attribute_names.inject({}) do |attrs, name|
          attrs[name] = read_attribute(name)
          attrs
        end
      end

      # Returns a hash of attributes before typecasting and deserialization.
      def attributes_before_type_cast
        self.attribute_names.inject({}) do |attrs, name|
          attrs[name] = read_attribute_before_type_cast(name)
          attrs
        end
      end

      # Returns an <tt>#inspect</tt>-like string for the value of the
      # attribute +attr_name+. String attributes are elided after 50
      # characters, and Date and Time attributes are returned in the
      # <tt>:db</tt> format. Other attributes return the value of
      # <tt>#inspect</tt> without modification.
      #
      #   person = Person.create!(:name => "David Heinemeier Hansson " * 3)
      #
      #   person.attribute_for_inspect(:name)
      #   # => '"David Heinemeier Hansson David Heinemeier Hansson D..."'
      #
      #   person.attribute_for_inspect(:created_at)
      #   # => '"2009-01-12 04:48:57"'
      def attribute_for_inspect(attr_name)
        value = read_attribute(attr_name)

        if value.is_a?(String) && value.length > 50
          "#{value[0..50]}...".inspect
        elsif value.is_a?(Date) || value.is_a?(Time)
          %("#{value.to_s(:db)}")
        else
          value.inspect
        end
      end

      # Returns true if the specified +attribute+ has been set by the user or by a database load and is neither
      # nil nor empty? (the latter only applies to objects that respond to empty?, most notably Strings).
      def attribute_present?(attribute)
        value = read_attribute(attribute)
        !value.blank?
      end

      # Returns true if the given attribute is in the attributes hash
      def has_attribute?(attr_name)
        @attributes.has_key?(attr_name.to_s)
      end

      # Returns an array of names for the attributes available on this object sorted alphabetically.
      def attribute_names
        @attributes.keys.sort
      end

      # Returns the column object for the named attribute.
      def column_for_attribute(name)
        self.class.columns_hash[name.to_s]
      end

      # Returns true if the +comparison_object+ is the same object, or is of the same type and has the same id.
      def ==(comparison_object)
        comparison_object.equal?(self) ||
          (comparison_object.instance_of?(self.class) &&
            comparison_object.id == id &&
            !comparison_object.new_record?)
      end

      # Delegates to ==
      def eql?(comparison_object)
        self == (comparison_object)
      end

      # Delegates to id in order to allow two records of the same type and id to work with something like:
      #   [ Person.find(1), Person.find(2), Person.find(3) ] & [ Person.find(1), Person.find(4) ] # => [ Person.find(1) ]
      def hash
        id.hash
      end

      # Freeze the attributes hash such that associations are still accessible, even on destroyed records.
      def freeze
        @attributes.freeze; self
      end

      # Returns +true+ if the attributes hash has been frozen.
      def frozen?
        @attributes.frozen?
      end

      # Returns +true+ if the record is read only. Records loaded through joins with piggy-back
      # attributes will be marked as read only since they cannot be saved.
      def readonly?
        defined?(@readonly) && @readonly == true
      end

      # Marks this record as read only.
      def readonly!
        @readonly = true
      end

      # Returns the contents of the record as a nicely formatted string.
      def inspect
        attributes_as_nice_string = self.class.column_names.collect { |name|
          if has_attribute?(name) || new_record?
            "#{name}: #{attribute_for_inspect(name)}"
          end
        }.compact.join(", ")
        "#<#{self.class} #{attributes_as_nice_string}>"
      end

    private
      def create_or_update
        raise ReadOnlyRecord if readonly?
        result = new_record? ? create : update
        result != false
      end

      # Updates the associated record with values matching those of the instance attributes.
      # Returns the number of affected rows.
      def update(attribute_names = @attributes.keys)
        quoted_attributes = attributes_with_quotes(false, false, attribute_names)
        return 0 if quoted_attributes.empty?
        connection.update(
          "UPDATE #{self.class.quoted_table_name} " +
          "SET #{quoted_comma_pair_list(connection, quoted_attributes)} " +
          "WHERE #{connection.quote_column_name(self.class.primary_key)} = #{quote_value(id)}",
          "#{self.class.name} Update"
        )
      end

      # Creates a record with values matching those of the instance attributes
      # and returns its id.
      def create
        if self.id.nil? && connection.prefetch_primary_key?(self.class.table_name)
          self.id = connection.next_sequence_value(self.class.sequence_name)
        end

        quoted_attributes = attributes_with_quotes

        statement = if quoted_attributes.empty?
          connection.empty_insert_statement(self.class.table_name)
        else
          "INSERT INTO #{self.class.quoted_table_name} " +
          "(#{quoted_column_names.join(', ')}) " +
          "VALUES(#{quoted_attributes.values.join(', ')})"
        end

        self.id = connection.insert(statement, "#{self.class.name} Create",
          self.class.primary_key, self.id, self.class.sequence_name)

        @new_record = false
        id
      end

      # Sets the attribute used for single table inheritance to this class name if this is not the ActiveRecord::Base descendant.
      # Considering the hierarchy Reply < Message < ActiveRecord::Base, this makes it possible to do Reply.new without having to
      # set <tt>Reply[Reply.inheritance_column] = "Reply"</tt> yourself. No such attribute would be set for objects of the
      # Message class in that example.
      def ensure_proper_type
        unless self.class.descends_from_active_record?
          write_attribute(self.class.inheritance_column, self.class.sti_name)
        end
      end

      def convert_number_column_value(value)
        if value == false
          0
        elsif value == true
          1
        elsif value.is_a?(String) && value.blank?
          nil
        else
          value
        end
      end

      def remove_attributes_protected_from_mass_assignment(attributes)
        safe_attributes =
          if self.class.accessible_attributes.nil? && self.class.protected_attributes.nil?
            attributes.reject { |key, value| attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          elsif self.class.protected_attributes.nil?
            attributes.reject { |key, value| !self.class.accessible_attributes.include?(key.gsub(/\(.+/, "")) || attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          elsif self.class.accessible_attributes.nil?
            attributes.reject { |key, value| self.class.protected_attributes.include?(key.gsub(/\(.+/,"")) || attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          else
            raise "Declare either attr_protected or attr_accessible for #{self.class}, but not both."
          end

        removed_attributes = attributes.keys - safe_attributes.keys

        if removed_attributes.any?
          log_protected_attribute_removal(removed_attributes)
        end

        safe_attributes
      end

      # Removes attributes which have been marked as readonly.
      def remove_readonly_attributes(attributes)
        unless self.class.readonly_attributes.nil?
          attributes.delete_if { |key, value| self.class.readonly_attributes.include?(key.gsub(/\(.+/,"")) }
        else
          attributes
        end
      end

      def log_protected_attribute_removal(*attributes)
        logger.debug "WARNING: Can't mass-assign these protected attributes: #{attributes.join(', ')}"
      end

      # The primary key and inheritance column can never be set by mass-assignment for security reasons.
      def attributes_protected_by_default
        default = [ self.class.primary_key, self.class.inheritance_column ]
        default << 'id' unless self.class.primary_key.eql? 'id'
        default
      end

      # Returns a copy of the attributes hash where all the values have been safely quoted for use in
      # an SQL statement.
      def attributes_with_quotes(include_primary_key = true, include_readonly_attributes = true, attribute_names = @attributes.keys)
        quoted = {}
        connection = self.class.connection
        attribute_names.each do |name|
          if (column = column_for_attribute(name)) && (include_primary_key || !column.primary)
            value = read_attribute(name)

            # We need explicit to_yaml because quote() does not properly convert Time/Date fields to YAML.
            if value && self.class.serialized_attributes.has_key?(name) && (value.acts_like?(:date) || value.acts_like?(:time))
              value = value.to_yaml
            end

            quoted[name] = connection.quote(value, column)
          end
        end
        include_readonly_attributes ? quoted : remove_readonly_attributes(quoted)
      end

      # Quote strings appropriately for SQL statements.
      def quote_value(value, column = nil)
        self.class.connection.quote(value, column)
      end

      # Interpolate custom SQL string in instance context.
      # Optional record argument is meant for custom insert_sql.
      def interpolate_sql(sql, record = nil)
        instance_eval("%@#{sql.gsub('@', '\@')}@")
      end

      # Initializes the attributes array with keys matching the columns from the linked table and
      # the values matching the corresponding default value of that column, so
      # that a new instance, or one populated from a passed-in Hash, still has all the attributes
      # that instances loaded from the database would.
      def attributes_from_column_definition
        self.class.columns.inject({}) do |attributes, column|
          attributes[column.name] = column.default unless column.name == self.class.primary_key
          attributes
        end
      end

      # Instantiates objects for all attribute classes that needs more than one constructor parameter. This is done
      # by calling new on the column type or aggregation type (through composed_of) object with these parameters.
      # So having the pairs written_on(1) = "2004", written_on(2) = "6", written_on(3) = "24", will instantiate
      # written_on (a date type) with Date.new("2004", "6", "24"). You can also specify a typecast character in the
      # parentheses to have the parameters typecasted before they're used in the constructor. Use i for Fixnum, f for Float,
      # s for String, and a for Array. If all the values for a given attribute are empty, the attribute will be set to nil.
      def assign_multiparameter_attributes(pairs)
        execute_callstack_for_multiparameter_attributes(
          extract_callstack_for_multiparameter_attributes(pairs)
        )
      end

      def instantiate_time_object(name, values)
        if self.class.send(:create_time_zone_conversion_attribute?, name, column_for_attribute(name))
          Time.zone.local(*values)
        else
          Time.time_with_datetime_fallback(@@default_timezone, *values)
        end
      end

      def execute_callstack_for_multiparameter_attributes(callstack)
        errors = []
        callstack.each do |name, values|
          begin
            klass = (self.class.reflect_on_aggregation(name.to_sym) || column_for_attribute(name)).klass
            if values.empty?
              send(name + "=", nil)
            else
              value = if Time == klass
                instantiate_time_object(name, values)
              elsif Date == klass
                begin
                  Date.new(*values)
                rescue ArgumentError => ex # if Date.new raises an exception on an invalid date
                  instantiate_time_object(name, values).to_date # we instantiate Time object and convert it back to a date thus using Time's logic in handling invalid dates
                end
              else
                klass.new(*values)
              end

              send(name + "=", value)
            end
          rescue => ex
            errors << AttributeAssignmentError.new("error on assignment #{values.inspect} to #{name}", ex, name)
          end
        end
        unless errors.empty?
          raise MultiparameterAssignmentErrors.new(errors), "#{errors.size} error(s) on assignment of multiparameter attributes"
        end
      end

      def extract_callstack_for_multiparameter_attributes(pairs)
        attributes = { }

        for pair in pairs
          multiparameter_name, value = pair
          attribute_name = multiparameter_name.split("(").first
          attributes[attribute_name] = [] unless attributes.include?(attribute_name)

          unless value.empty?
            attributes[attribute_name] <<
              [ find_parameter_position(multiparameter_name), type_cast_attribute_value(multiparameter_name, value) ]
          end
        end

        attributes.each { |name, values| attributes[name] = values.sort_by{ |v| v.first }.collect { |v| v.last } }
      end

      def type_cast_attribute_value(multiparameter_name, value)
        multiparameter_name =~ /\([0-9]*([if])\)/ ? value.send("to_" + $1) : value
      end

      def find_parameter_position(multiparameter_name)
        multiparameter_name.scan(/\(([0-9]*).*\)/).first.first
      end

      # Returns a comma-separated pair list, like "key1 = val1, key2 = val2".
      def comma_pair_list(hash)
        hash.inject([]) { |list, pair| list << "#{pair.first} = #{pair.last}" }.join(", ")
      end

      def quoted_column_names(attributes = attributes_with_quotes)
        connection = self.class.connection
        attributes.keys.collect do |column_name|
          connection.quote_column_name(column_name)
        end
      end

      def self.quoted_table_name
        self.connection.quote_table_name(self.table_name)
      end

      def quote_columns(quoter, hash)
        hash.inject({}) do |quoted, (name, value)|
          quoted[quoter.quote_column_name(name)] = value
          quoted
        end
      end

      def quoted_comma_pair_list(quoter, hash)
        comma_pair_list(quote_columns(quoter, hash))
      end

      def object_from_yaml(string)
        return string unless string.is_a?(String) && string =~ /^---/
        YAML::load(string) rescue string
      end

      def clone_attributes(reader_method = :read_attribute, attributes = {})
        self.attribute_names.inject(attributes) do |attrs, name|
          attrs[name] = clone_attribute_value(reader_method, name)
          attrs
        end
      end

      def clone_attribute_value(reader_method, attribute_name)
        value = send(reader_method, attribute_name)
        value.duplicable? ? value.clone : value
      rescue TypeError, NoMethodError
        value
      end
  end

  Base.class_eval do
    extend QueryCache::ClassMethods
    include Validations
    include Locking::Optimistic, Locking::Pessimistic
    include AttributeMethods
    include Dirty
    include Callbacks, Observing, Timestamp
    include Associations, AssociationPreload, NamedScope

    # AutosaveAssociation needs to be included before Transactions, because we want
    # #save_with_autosave_associations to be wrapped inside a transaction.
    include AutosaveAssociation, NestedAttributes

    include Aggregations, Transactions, Reflection, Batches, Calculations, Serialization
  end
end

# TODO: Remove this and make it work with LAZY flag
require 'active_record/connection_adapters/abstract_adapter'
            respond_to?(:"#{k}=") ? send(:"#{k}=", v) : raise(UnknownAttributeError, "unknown attribute: #{k}")
          end
        end

        assign_multiparameter_attributes(multi_parameter_attributes)
      end

      # Returns a hash of all the attributes with their names as keys and the values of the attributes as values.
      def attributes
        self.attribute_names.inject({}) do |attrs, name|
          attrs[name] = read_attribute(name)
          attrs
        end
      end

      # Returns a hash of attributes before typecasting and deserialization.
      def attributes_before_type_cast
        self.attribute_names.inject({}) do |attrs, name|
          attrs[name] = read_attribute_before_type_cast(name)
          attrs
        end
      end

      # Returns an <tt>#inspect</tt>-like string for the value of the
      # attribute +attr_name+. String attributes are elided after 50
      # characters, and Date and Time attributes are returned in the
      # <tt>:db</tt> format. Other attributes return the value of
      # <tt>#inspect</tt> without modification.
      #
      #   person = Person.create!(:name => "David Heinemeier Hansson " * 3)
      #
      #   person.attribute_for_inspect(:name)
      #   # => '"David Heinemeier Hansson David Heinemeier Hansson D..."'
      #
      #   person.attribute_for_inspect(:created_at)
      #   # => '"2009-01-12 04:48:57"'
      def attribute_for_inspect(attr_name)
        value = read_attribute(attr_name)

        if value.is_a?(String) && value.length > 50
          "#{value[0..50]}...".inspect
        elsif value.is_a?(Date) || value.is_a?(Time)
          %("#{value.to_s(:db)}")
        else
          value.inspect
        end
      end

      # Returns true if the specified +attribute+ has been set by the user or by a database load and is neither
      # nil nor empty? (the latter only applies to objects that respond to empty?, most notably Strings).
      def attribute_present?(attribute)
        value = read_attribute(attribute)
        !value.blank?
      end

      # Returns true if the given attribute is in the attributes hash
      def has_attribute?(attr_name)
        @attributes.has_key?(attr_name.to_s)
      end

      # Returns an array of names for the attributes available on this object sorted alphabetically.
      def attribute_names
        @attributes.keys.sort
      end

      # Returns the column object for the named attribute.
      def column_for_attribute(name)
        self.class.columns_hash[name.to_s]
      end

      # Returns true if the +comparison_object+ is the same object, or is of the same type and has the same id.
      def ==(comparison_object)
        comparison_object.equal?(self) ||
          (comparison_object.instance_of?(self.class) &&
            comparison_object.id == id &&
            !comparison_object.new_record?)
      end

      # Delegates to ==
      def eql?(comparison_object)
        self == (comparison_object)
      end

      # Delegates to id in order to allow two records of the same type and id to work with something like:
      #   [ Person.find(1), Person.find(2), Person.find(3) ] & [ Person.find(1), Person.find(4) ] # => [ Person.find(1) ]
      def hash
        id.hash
      end

      # Freeze the attributes hash such that associations are still accessible, even on destroyed records.
      def freeze
        @attributes.freeze; self
      end

      # Returns +true+ if the attributes hash has been frozen.
      def frozen?
        @attributes.frozen?
      end

      # Returns +true+ if the record is read only. Records loaded through joins with piggy-back
      # attributes will be marked as read only since they cannot be saved.
      def readonly?
        defined?(@readonly) && @readonly == true
      end

      # Marks this record as read only.
      def readonly!
        @readonly = true
      end

      # Returns the contents of the record as a nicely formatted string.
      def inspect
        attributes_as_nice_string = self.class.column_names.collect { |name|
          if has_attribute?(name) || new_record?
            "#{name}: #{attribute_for_inspect(name)}"
          end
        }.compact.join(", ")
        "#<#{self.class} #{attributes_as_nice_string}>"
      end

    private
      def create_or_update
        raise ReadOnlyRecord if readonly?
        result = new_record? ? create : update
        result != false
      end

      # Updates the associated record with values matching those of the instance attributes.
      # Returns the number of affected rows.
      def update(attribute_names = @attributes.keys)
        quoted_attributes = attributes_with_quotes(false, false, attribute_names)
        return 0 if quoted_attributes.empty?
        connection.update(
          "UPDATE #{self.class.quoted_table_name} " +
          "SET #{quoted_comma_pair_list(connection, quoted_attributes)} " +
          "WHERE #{connection.quote_column_name(self.class.primary_key)} = #{quote_value(id)}",
          "#{self.class.name} Update"
        )
      end

      # Creates a record with values matching those of the instance attributes
      # and returns its id.
      def create
        if self.id.nil? && connection.prefetch_primary_key?(self.class.table_name)
          self.id = connection.next_sequence_value(self.class.sequence_name)
        end

        quoted_attributes = attributes_with_quotes

        statement = if quoted_attributes.empty?
          connection.empty_insert_statement(self.class.table_name)
        else
          "INSERT INTO #{self.class.quoted_table_name} " +
          "(#{quoted_column_names.join(', ')}) " +
          "VALUES(#{quoted_attributes.values.join(', ')})"
        end

        self.id = connection.insert(statement, "#{self.class.name} Create",
          self.class.primary_key, self.id, self.class.sequence_name)

        @new_record = false
        id
      end

      # Sets the attribute used for single table inheritance to this class name if this is not the ActiveRecord::Base descendant.
      # Considering the hierarchy Reply < Message < ActiveRecord::Base, this makes it possible to do Reply.new without having to
      # set <tt>Reply[Reply.inheritance_column] = "Reply"</tt> yourself. No such attribute would be set for objects of the
      # Message class in that example.
      def ensure_proper_type
        unless self.class.descends_from_active_record?
          write_attribute(self.class.inheritance_column, self.class.sti_name)
        end
      end

      def convert_number_column_value(value)
        if value == false
          0
        elsif value == true
          1
        elsif value.is_a?(String) && value.blank?
          nil
        else
          value
        end
      end

      def remove_attributes_protected_from_mass_assignment(attributes)
        safe_attributes =
          if self.class.accessible_attributes.nil? && self.class.protected_attributes.nil?
            attributes.reject { |key, value| attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          elsif self.class.protected_attributes.nil?
            attributes.reject { |key, value| !self.class.accessible_attributes.include?(key.gsub(/\(.+/, "")) || attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          elsif self.class.accessible_attributes.nil?
            attributes.reject { |key, value| self.class.protected_attributes.include?(key.gsub(/\(.+/,"")) || attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          else
            raise "Declare either attr_protected or attr_accessible for #{self.class}, but not both."
          end

        removed_attributes = attributes.keys - safe_attributes.keys

        if removed_attributes.any?
          log_protected_attribute_removal(removed_attributes)
        end

        safe_attributes
      end

      # Removes attributes which have been marked as readonly.
      def remove_readonly_attributes(attributes)
        unless self.class.readonly_attributes.nil?
          attributes.delete_if { |key, value| self.class.readonly_attributes.include?(key.gsub(/\(.+/,"")) }
        else
          attributes
        end
      end

      def log_protected_attribute_removal(*attributes)
        logger.debug "WARNING: Can't mass-assign these protected attributes: #{attributes.join(', ')}"
      end

      # The primary key and inheritance column can never be set by mass-assignment for security reasons.
      def attributes_protected_by_default
        default = [ self.class.primary_key, self.class.inheritance_column ]
        default << 'id' unless self.class.primary_key.eql? 'id'
        default
      end

      # Returns a copy of the attributes hash where all the values have been safely quoted for use in
      # an SQL statement.
      def attributes_with_quotes(include_primary_key = true, include_readonly_attributes = true, attribute_names = @attributes.keys)
        quoted = {}
        connection = self.class.connection
        attribute_names.each do |name|
          if (column = column_for_attribute(name)) && (include_primary_key || !column.primary)
            value = read_attribute(name)

            # We need explicit to_yaml because quote() does not properly convert Time/Date fields to YAML.
            if value && self.class.serialized_attributes.has_key?(name) && (value.acts_like?(:date) || value.acts_like?(:time))
              value = value.to_yaml
            end

            quoted[name] = connection.quote(value, column)
          end
        end
        include_readonly_attributes ? quoted : remove_readonly_attributes(quoted)
      end

      # Quote strings appropriately for SQL statements.
      def quote_value(value, column = nil)
        self.class.connection.quote(value, column)
      end

      # Interpolate custom SQL string in instance context.
      # Optional record argument is meant for custom insert_sql.
      def interpolate_sql(sql, record = nil)
        instance_eval("%@#{sql.gsub('@', '\@')}@")
      end

      # Initializes the attributes array with keys matching the columns from the linked table and
      # the values matching the corresponding default value of that column, so
      # that a new instance, or one populated from a passed-in Hash, still has all the attributes
      # that instances loaded from the database would.
      def attributes_from_column_definition
        self.class.columns.inject({}) do |attributes, column|
          attributes[column.name] = column.default unless column.name == self.class.primary_key
          attributes
        end
      end

      # Instantiates objects for all attribute classes that needs more than one constructor parameter. This is done
      # by calling new on the column type or aggregation type (through composed_of) object with these parameters.
      # So having the pairs written_on(1) = "2004", written_on(2) = "6", written_on(3) = "24", will instantiate
      # written_on (a date type) with Date.new("2004", "6", "24"). You can also specify a typecast character in the
      # parentheses to have the parameters typecasted before they're used in the constructor. Use i for Fixnum, f for Float,
      # s for String, and a for Array. If all the values for a given attribute are empty, the attribute will be set to nil.
      def assign_multiparameter_attributes(pairs)
        execute_callstack_for_multiparameter_attributes(
          extract_callstack_for_multiparameter_attributes(pairs)
        )
      end

      def instantiate_time_object(name, values)
        if self.class.send(:create_time_zone_conversion_attribute?, name, column_for_attribute(name))
          Time.zone.local(*values)
        else
          Time.time_with_datetime_fallback(@@default_timezone, *values)
        end
      end

      def execute_callstack_for_multiparameter_attributes(callstack)
        errors = []
        callstack.each do |name, values|
          begin
            klass = (self.class.reflect_on_aggregation(name.to_sym) || column_for_attribute(name)).klass
            if values.empty?
              send(name + "=", nil)
            else
              value = if Time == klass
                instantiate_time_object(name, values)
              elsif Date == klass
                begin
                  Date.new(*values)
                rescue ArgumentError => ex # if Date.new raises an exception on an invalid date
                  instantiate_time_object(name, values).to_date # we instantiate Time object and convert it back to a date thus using Time's logic in handling invalid dates
                end
              else
                klass.new(*values)
              end

              send(name + "=", value)
            end
          rescue => ex
            errors << AttributeAssignmentError.new("error on assignment #{values.inspect} to #{name}", ex, name)
          end
        end
        unless errors.empty?
          raise MultiparameterAssignmentErrors.new(errors), "#{errors.size} error(s) on assignment of multiparameter attributes"
        end
      end

      def extract_callstack_for_multiparameter_attributes(pairs)
        attributes = { }

        for pair in pairs
          multiparameter_name, value = pair
          attribute_name = multiparameter_name.split("(").first
          attributes[attribute_name] = [] unless attributes.include?(attribute_name)

          unless value.empty?
            attributes[attribute_name] <<
              [ find_parameter_position(multiparameter_name), type_cast_attribute_value(multiparameter_name, value) ]
          end
        end

        attributes.each { |name, values| attributes[name] = values.sort_by{ |v| v.first }.collect { |v| v.last } }
      end

      def type_cast_attribute_value(multiparameter_name, value)
        multiparameter_name =~ /\([0-9]*([if])\)/ ? value.send("to_" + $1) : value
      end

      def find_parameter_position(multiparameter_name)
        multiparameter_name.scan(/\(([0-9]*).*\)/).first.first
      end

      # Returns a comma-separated pair list, like "key1 = val1, key2 = val2".
      def comma_pair_list(hash)
        hash.inject([]) { |list, pair| list << "#{pair.first} = #{pair.last}" }.join(", ")
      end

      def quoted_column_names(attributes = attributes_with_quotes)
        connection = self.class.connection
        attributes.keys.collect do |column_name|
          connection.quote_column_name(column_name)
        end
      end

      def self.quoted_table_name
        self.connection.quote_table_name(self.table_name)
      end

      def quote_columns(quoter, hash)
        hash.inject({}) do |quoted, (name, value)|
          quoted[quoter.quote_column_name(name)] = value
          quoted
        end
      end

      def quoted_comma_pair_list(quoter, hash)
        comma_pair_list(quote_columns(quoter, hash))
      end

      def object_from_yaml(string)
        return string unless string.is_a?(String) && string =~ /^---/
        YAML::load(string) rescue string
      end

      def clone_attributes(reader_method = :read_attribute, attributes = {})
        self.attribute_names.inject(attributes) do |attrs, name|
          attrs[name] = clone_attribute_value(reader_method, name)
          attrs
        end
      end

      def clone_attribute_value(reader_method, attribute_name)
        value = send(reader_method, attribute_name)
        value.duplicable? ? value.clone : value
      rescue TypeError, NoMethodError
        value
      end
  end

  Base.class_eval do
    extend QueryCache::ClassMethods
    include Validations
    include Locking::Optimistic, Locking::Pessimistic
    include AttributeMethods
    include Dirty
    include Callbacks, Observing, Timestamp
    include Associations, AssociationPreload, NamedScope

    # AutosaveAssociation needs to be included before Transactions, because we want
    # #save_with_autosave_associations to be wrapped inside a transaction.
    include AutosaveAssociation, NestedAttributes

    include Aggregations, Transactions, Reflection, Batches, Calculations, Serialization
  end
end

# TODO: Remove this and make it work with LAZY flag
require 'active_record/connection_adapters/abstract_adapter'
            respond_to?(:"#{k}=") ? send(:"#{k}=", v) : raise(UnknownAttributeError, "unknown attribute: #{k}")
          end
        end

        assign_multiparameter_attributes(multi_parameter_attributes)
      end

      # Returns a hash of all the attributes with their names as keys and the values of the attributes as values.
      def attributes
        self.attribute_names.inject({}) do |attrs, name|
          attrs[name] = read_attribute(name)
          attrs
        end
      end

      # Returns a hash of attributes before typecasting and deserialization.
      def attributes_before_type_cast
        self.attribute_names.inject({}) do |attrs, name|
          attrs[name] = read_attribute_before_type_cast(name)
          attrs
        end
      end

      # Returns an <tt>#inspect</tt>-like string for the value of the
      # attribute +attr_name+. String attributes are elided after 50
      # characters, and Date and Time attributes are returned in the
      # <tt>:db</tt> format. Other attributes return the value of
      # <tt>#inspect</tt> without modification.
      #
      #   person = Person.create!(:name => "David Heinemeier Hansson " * 3)
      #
      #   person.attribute_for_inspect(:name)
      #   # => '"David Heinemeier Hansson David Heinemeier Hansson D..."'
      #
      #   person.attribute_for_inspect(:created_at)
      #   # => '"2009-01-12 04:48:57"'
      def attribute_for_inspect(attr_name)
        value = read_attribute(attr_name)

        if value.is_a?(String) && value.length > 50
          "#{value[0..50]}...".inspect
        elsif value.is_a?(Date) || value.is_a?(Time)
          %("#{value.to_s(:db)}")
        else
          value.inspect
        end
      end

      # Returns true if the specified +attribute+ has been set by the user or by a database load and is neither
      # nil nor empty? (the latter only applies to objects that respond to empty?, most notably Strings).
      def attribute_present?(attribute)
        value = read_attribute(attribute)
        !value.blank?
      end

      # Returns true if the given attribute is in the attributes hash
      def has_attribute?(attr_name)
        @attributes.has_key?(attr_name.to_s)
      end

      # Returns an array of names for the attributes available on this object sorted alphabetically.
      def attribute_names
        @attributes.keys.sort
      end

      # Returns the column object for the named attribute.
      def column_for_attribute(name)
        self.class.columns_hash[name.to_s]
      end

      # Returns true if the +comparison_object+ is the same object, or is of the same type and has the same id.
      def ==(comparison_object)
        comparison_object.equal?(self) ||
          (comparison_object.instance_of?(self.class) &&
            comparison_object.id == id &&
            !comparison_object.new_record?)
      end

      # Delegates to ==
      def eql?(comparison_object)
        self == (comparison_object)
      end

      # Delegates to id in order to allow two records of the same type and id to work with something like:
      #   [ Person.find(1), Person.find(2), Person.find(3) ] & [ Person.find(1), Person.find(4) ] # => [ Person.find(1) ]
      def hash
        id.hash
      end

      # Freeze the attributes hash such that associations are still accessible, even on destroyed records.
      def freeze
        @attributes.freeze; self
      end

      # Returns +true+ if the attributes hash has been frozen.
      def frozen?
        @attributes.frozen?
      end

      # Returns +true+ if the record is read only. Records loaded through joins with piggy-back
      # attributes will be marked as read only since they cannot be saved.
      def readonly?
        defined?(@readonly) && @readonly == true
      end

      # Marks this record as read only.
      def readonly!
        @readonly = true
      end

      # Returns the contents of the record as a nicely formatted string.
      def inspect
        attributes_as_nice_string = self.class.column_names.collect { |name|
          if has_attribute?(name) || new_record?
            "#{name}: #{attribute_for_inspect(name)}"
          end
        }.compact.join(", ")
        "#<#{self.class} #{attributes_as_nice_string}>"
      end

    private
      def create_or_update
        raise ReadOnlyRecord if readonly?
        result = new_record? ? create : update
        result != false
      end

      # Updates the associated record with values matching those of the instance attributes.
      # Returns the number of affected rows.
      def update(attribute_names = @attributes.keys)
        quoted_attributes = attributes_with_quotes(false, false, attribute_names)
        return 0 if quoted_attributes.empty?
        connection.update(
          "UPDATE #{self.class.quoted_table_name} " +
          "SET #{quoted_comma_pair_list(connection, quoted_attributes)} " +
          "WHERE #{connection.quote_column_name(self.class.primary_key)} = #{quote_value(id)}",
          "#{self.class.name} Update"
        )
      end

      # Creates a record with values matching those of the instance attributes
      # and returns its id.
      def create
        if self.id.nil? && connection.prefetch_primary_key?(self.class.table_name)
          self.id = connection.next_sequence_value(self.class.sequence_name)
        end

        quoted_attributes = attributes_with_quotes

        statement = if quoted_attributes.empty?
          connection.empty_insert_statement(self.class.table_name)
        else
          "INSERT INTO #{self.class.quoted_table_name} " +
          "(#{quoted_column_names.join(', ')}) " +
          "VALUES(#{quoted_attributes.values.join(', ')})"
        end

        self.id = connection.insert(statement, "#{self.class.name} Create",
          self.class.primary_key, self.id, self.class.sequence_name)

        @new_record = false
        id
      end

      # Sets the attribute used for single table inheritance to this class name if this is not the ActiveRecord::Base descendant.
      # Considering the hierarchy Reply < Message < ActiveRecord::Base, this makes it possible to do Reply.new without having to
      # set <tt>Reply[Reply.inheritance_column] = "Reply"</tt> yourself. No such attribute would be set for objects of the
      # Message class in that example.
      def ensure_proper_type
        unless self.class.descends_from_active_record?
          write_attribute(self.class.inheritance_column, self.class.sti_name)
        end
      end

      def convert_number_column_value(value)
        if value == false
          0
        elsif value == true
          1
        elsif value.is_a?(String) && value.blank?
          nil
        else
          value
        end
      end

      def remove_attributes_protected_from_mass_assignment(attributes)
        safe_attributes =
          if self.class.accessible_attributes.nil? && self.class.protected_attributes.nil?
            attributes.reject { |key, value| attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          elsif self.class.protected_attributes.nil?
            attributes.reject { |key, value| !self.class.accessible_attributes.include?(key.gsub(/\(.+/, "")) || attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          elsif self.class.accessible_attributes.nil?
            attributes.reject { |key, value| self.class.protected_attributes.include?(key.gsub(/\(.+/,"")) || attributes_protected_by_default.include?(key.gsub(/\(.+/, "")) }
          else
            raise "Declare either attr_protected or attr_accessible for #{self.class}, but not both."
          end

        removed_attributes = attributes.keys - safe_attributes.keys

        if removed_attributes.any?
          log_protected_attribute_removal(removed_attributes)
        end

        safe_attributes
      end

      # Removes attributes which have been marked as readonly.
      def remove_readonly_attributes(attributes)
        unless self.class.readonly_attributes.nil?
          attributes.delete_if { |key, value| self.class.readonly_attributes.include?(key.gsub(/\(.+/,"")) }
        else
          attributes
        end
      end

      def log_protected_attribute_removal(*attributes)
        logger.debug "WARNING: Can't mass-assign these protected attributes: #{attributes.join(', ')}"
      end

      # The primary key and inheritance column can never be set by mass-assignment for security reasons.
      def attributes_protected_by_default
        default = [ self.class.primary_key, self.class.inheritance_column ]
        default << 'id' unless self.class.primary_key.eql? 'id'
        default
      end

      # Returns a copy of the attributes hash where all the values have been safely quoted for use in
      # an SQL statement.
      def attributes_with_quotes(include_primary_key = true, include_readonly_attributes = true, attribute_names = @attributes.keys)
        quoted = {}
        connection = self.class.connection
        attribute_names.each do |name|
          if (column = column_for_attribute(name)) && (include_primary_key || !column.primary)
            value = read_attribute(name)

            # We need explicit to_yaml because quote() does not properly convert Time/Date fields to YAML.
            if value && self.class.serialized_attributes.has_key?(name) && (value.acts_like?(:date) || value.acts_like?(:time))
              value = value.to_yaml
            end

            quoted[name] = connection.quote(value, column)
          end
        end
        include_readonly_attributes ? quoted : remove_readonly_attributes(quoted)
      end

      # Quote strings appropriately for SQL statements.
      def quote_value(value, column = nil)
        self.class.connection.quote(value, column)
      end

      # Interpolate custom SQL string in instance context.
      # Optional record argument is meant for custom insert_sql.
      def interpolate_sql(sql, record = nil)
        instance_eval("%@#{sql.gsub('@', '\@')}@")
      end

      # Initializes the attributes array with keys matching the columns from the linked table and
      # the values matching the corresponding default value of that column, so
      # that a new instance, or one populated from a passed-in Hash, still has all the attributes
      # that instances loaded from the database would.
      def attributes_from_column_definition
        self.class.columns.inject({}) do |attributes, column|
          attributes[column.name] = column.default unless column.name == self.class.primary_key
          attributes
        end
      end

      # Instantiates objects for all attribute classes that needs more than one constructor parameter. This is done
      # by calling new on the column type or aggregation type (through composed_of) object with these parameters.
      # So having the pairs written_on(1) = "2004", written_on(2) = "6", written_on(3) = "24", will instantiate
      # written_on (a date type) with Date.new("2004", "6", "24"). You can also specify a typecast character in the
      # parentheses to have the parameters typecasted before they're used in the constructor. Use i for Fixnum, f for Float,
      # s for String, and a for Array. If all the values for a given attribute are empty, the attribute will be set to nil.
      def assign_multiparameter_attributes(pairs)
        execute_callstack_for_multiparameter_attributes(
          extract_callstack_for_multiparameter_attributes(pairs)
        )
      end

      def instantiate_time_object(name, values)
        if self.class.send(:create_time_zone_conversion_attribute?, name, column_for_attribute(name))
          Time.zone.local(*values)
        else
          Time.time_with_datetime_fallback(@@default_timezone, *values)
        end
      end

      def execute_callstack_for_multiparameter_attributes(callstack)
        errors = []
        callstack.each do |name, values|
          begin
            klass = (self.class.reflect_on_aggregation(name.to_sym) || column_for_attribute(name)).klass
            if values.empty?
              send(name + "=", nil)
            else
              value = if Time == klass
                instantiate_time_object(name, values)
              elsif Date == klass
                begin
                  Date.new(*values)
                rescue ArgumentError => ex # if Date.new raises an exception on an invalid date
                  instantiate_time_object(name, values).to_date # we instantiate Time object and convert it back to a date thus using Time's logic in handling invalid dates
                end
              else
                klass.new(*values)
              end

              send(name + "=", value)
            end
          rescue => ex
            errors << AttributeAssignmentError.new("error on assignment #{values.inspect} to #{name}", ex, name)
          end
        end
        unless errors.empty?
          raise MultiparameterAssignmentErrors.new(errors), "#{errors.size} error(s) on assignment of multiparameter attributes"
        end
      end

      def extract_callstack_for_multiparameter_attributes(pairs)
        attributes = { }

        for pair in pairs
          multiparameter_name, value = pair
          attribute_name = multiparameter_name.split("(").first
          attributes[attribute_name] = [] unless attributes.include?(attribute_name)

          unless value.empty?
            attributes[attribute_name] <<
              [ find_parameter_position(multiparameter_name), type_cast_attribute_value(multiparameter_name, value) ]
          end
        end

        attributes.each { |name, values| attributes[name] = values.sort_by{ |v| v.first }.collect { |v| v.last } }
      end

      def type_cast_attribute_value(multiparameter_name, value)
        multiparameter_name =~ /\([0-9]*([if])\)/ ? value.send("to_" + $1) : value
      end

      def find_parameter_position(multiparameter_name)
        multiparameter_name.scan(/\(([0-9]*).*\)/).first.first
      end

      # Returns a comma-separated pair list, like "key1 = val1, key2 = val2".
      def comma_pair_list(hash)
        hash.inject([]) { |list, pair| list << "#{pair.first} = #{pair.last}" }.join(", ")
      end

      def quoted_column_names(attributes = attributes_with_quotes)
        connection = self.class.connection
        attributes.keys.collect do |column_name|
          connection.quote_column_name(column_name)
        end
      end

      def self.quoted_table_name
        self.connection.quote_table_name(self.table_name)
      end

      def quote_columns(quoter, hash)
        hash.inject({}) do |quoted, (name, value)|
          quoted[quoter.quote_column_name(name)] = value
          quoted
        end
      end

      def quoted_comma_pair_list(quoter, hash)
        comma_pair_list(quote_columns(quoter, hash))
      end

      def object_from_yaml(string)
        return string unless string.is_a?(String) && string =~ /^---/
        YAML::load(string) rescue string
      end

      def clone_attributes(reader_method = :read_attribute, attributes = {})
        self.attribute_names.inject(attributes) do |attrs, name|
          attrs[name] = clone_attribute_value(reader_method, name)
          attrs
        end
      end

      def clone_attribute_value(reader_method, attribute_name)
        value = send(reader_method, attribute_name)
        value.duplicable? ? value.clone : value
      rescue TypeError, NoMethodError
        value
      end
  end

  Base.class_eval do
    extend QueryCache::ClassMethods
    include Validations
    include Locking::Optimistic, Locking::Pessimistic
    include AttributeMethods
    include Dirty
    include Callbacks, Observing, Timestamp
    include Associations, AssociationPreload, NamedScope

    # AutosaveAssociation needs to be included before Transactions, because we want
    # #save_with_autosave_associations to be wrapped inside a transaction.
    include AutosaveAssociation, NestedAttributes

    include Aggregations, Transactions, Reflection, Batches, Calculations, Serialization
  end
end

# TODO: Remove this and make it work with LAZY flag
require 'active_record/connection_adapters/abstract_adapter'