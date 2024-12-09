    # Example (for PostgreSQL with pg_hint_plan):
    #
    #   Topic.optimizer_hints("SeqScan(topics)", "Parallel(topics 8)")
    #   # SELECT /*+ SeqScan(topics) Parallel(topics 8) */ "topics".* FROM "topics"
    def optimizer_hints(*args)
      check_if_method_has_arguments!(__callee__, args)
      spawn.optimizer_hints!(*args)
    end

    def optimizer_hints!(*args) # :nodoc:
      self.optimizer_hints_values |= args
      self
    end

    # Reverse the existing order clause on the relation.
    #
    #   User.order('name ASC').reverse_order # generated SQL has 'ORDER BY name DESC'
    def reverse_order
      spawn.reverse_order!
    end

    def reverse_order! # :nodoc:
      orders = order_values.compact_blank
      self.order_values = reverse_sql_order(orders)
      self
    end

    def skip_query_cache!(value = true) # :nodoc:
      self.skip_query_cache_value = value
      self
    end

    def skip_preloading! # :nodoc:
      self.skip_preloading_value = true
      self
    end

    # Adds an SQL comment to queries generated from this relation. For example:
    #
    #   User.annotate("selecting user names").select(:name)
    #   # SELECT "users"."name" FROM "users" /* selecting user names */
    #
    #   User.annotate("selecting", "user", "names").select(:name)
    #   # SELECT "users"."name" FROM "users" /* selecting */ /* user */ /* names */
    #
    # The SQL block comment delimiters, "/*" and "*/", will be added automatically.
    #
    # Some escaping is performed, however untrusted user input should not be used.
    def annotate(*args)
      check_if_method_has_arguments!(__callee__, args)
      spawn.annotate!(*args)
    end

    # Like #annotate, but modifies relation in place.
    def annotate!(*args) # :nodoc:
      self.annotate_values += args
      self
    end

    # Deduplicate multiple values.
    def uniq!(name)
      if values = @values[name]
        values.uniq! if values.is_a?(Array) && !values.empty?
      end
      self
    end

    # Excludes the specified record (or collection of records) from the resulting
    # relation. For example:
    #
    #   Post.excluding(post)
    #   # SELECT "posts".* FROM "posts" WHERE "posts"."id" != 1
    #
    #   Post.excluding(post_one, post_two)
    #   # SELECT "posts".* FROM "posts" WHERE "posts"."id" NOT IN (1, 2)
    #
    # This can also be called on associations. As with the above example, either
    # a single record of collection thereof may be specified:
    #
    #   post = Post.find(1)
    #   comment = Comment.find(2)
    #   post.comments.excluding(comment)
    #   # SELECT "comments".* FROM "comments" WHERE "comments"."post_id" = 1 AND "comments"."id" != 2
    #
    # This is short-hand for <tt>.where.not(id: post.id)</tt> and <tt>.where.not(id: [post_one.id, post_two.id])</tt>.
    #
    # An <tt>ArgumentError</tt> will be raised if either no records are
    # specified, or if any of the records in the collection (if a collection
    # is passed in) are not instances of the same model that the relation is
    # scoping.
    def excluding(*records)
      records.flatten!(1)
      records.compact!

      unless records.all?(klass)
        raise ArgumentError, "You must only pass a single or collection of #{klass.name} objects to ##{__callee__}."
      end

      spawn.excluding!(records)
    end
    alias :without :excluding

    def excluding!(records) # :nodoc:
      predicates = [ predicate_builder[primary_key, records].invert ]
      self.where_clause += Relation::WhereClause.new(predicates)
      self
    end

    # Returns the Arel object associated with the relation.
    def arel(aliases = nil) # :nodoc:
      @arel ||= build_arel(aliases)
    end

    def construct_join_dependency(associations, join_type) # :nodoc:
      ActiveRecord::Associations::JoinDependency.new(
        klass, table, associations, join_type
      )
    end

    protected
      def build_subquery(subquery_alias, select_value) # :nodoc:
        subquery = except(:optimizer_hints).arel.as(subquery_alias)

        Arel::SelectManager.new(subquery).project(select_value).tap do |arel|
          arel.optimizer_hints(*optimizer_hints_values) unless optimizer_hints_values.empty?
        end
      end

      def build_where_clause(opts, rest = []) # :nodoc:
        opts = sanitize_forbidden_attributes(opts)

        case opts
        when String, Array
          parts = [klass.sanitize_sql(rest.empty? ? opts : [opts, *rest])]
        when Hash
          opts = opts.transform_keys do |key|
            key = key.to_s
            klass.attribute_aliases[key] || key
          end
          references = PredicateBuilder.references(opts)
          self.references_values |= references unless references.empty?

          parts = predicate_builder.build_from_hash(opts) do |table_name|
            lookup_table_klass_from_join_dependencies(table_name)
          end
        when Arel::Nodes::Node
          parts = [opts]
        else
          raise ArgumentError, "Unsupported argument type: #{opts} (#{opts.class})"
        end

        Relation::WhereClause.new(parts)
      end
      alias :build_having_clause :build_where_clause

      def async!
        @async = true
        self
      end

    private
      def async
        spawn.async!
      end

      def lookup_table_klass_from_join_dependencies(table_name)
        each_join_dependencies do |join|
          return join.base_klass if table_name == join.table_name
        end
        nil
      end

      def each_join_dependencies(join_dependencies = build_join_dependencies, &block)
        join_dependencies.each do |join_dependency|
          join_dependency.each(&block)
        end
      end

      def build_join_dependencies
        associations = joins_values | left_outer_joins_values
        associations |= eager_load_values unless eager_load_values.empty?
        associations |= includes_values unless includes_values.empty?

        join_dependencies = []
        join_dependencies.unshift construct_join_dependency(
          select_association_list(associations, join_dependencies), nil
        )
      end

      def assert_mutability!
        raise ImmutableRelation if @loaded
        raise ImmutableRelation if defined?(@arel) && @arel
      end

      def build_arel(aliases = nil)
        arel = Arel::SelectManager.new(table)

        build_joins(arel.join_sources, aliases)

        arel.where(where_clause.ast) unless where_clause.empty?
        arel.having(having_clause.ast) unless having_clause.empty?
        arel.take(build_cast_value("LIMIT", connection.sanitize_limit(limit_value))) if limit_value
        arel.skip(build_cast_value("OFFSET", offset_value.to_i)) if offset_value
        arel.group(*arel_columns(group_values.uniq)) unless group_values.empty?

        build_order(arel)
        build_with(arel)
        build_select(arel)

        arel.optimizer_hints(*optimizer_hints_values) unless optimizer_hints_values.empty?
        arel.distinct(distinct_value)
        arel.from(build_from) unless from_clause.empty?
        arel.lock(lock_value) if lock_value

        unless annotate_values.empty?
          annotates = annotate_values
          annotates = annotates.uniq if annotates.size > 1
          arel.comment(*annotates)
        end

        arel
      end

      def build_cast_value(name, value)
        ActiveModel::Attribute.with_cast_value(name, value, Type.default_value)
      end

      def build_from
        opts = from_clause.value
        name = from_clause.name
        case opts
        when Relation
          if opts.eager_loading?
            opts = opts.send(:apply_join_dependency)
          end
          name ||= "subquery"
          opts.arel.as(name.to_s)
        else
          opts
        end
      end

      def select_association_list(associations, stashed_joins = nil)
        result = []
        associations.each do |association|
          case association
          when Hash, Symbol, Array
            result << association
          when ActiveRecord::Associations::JoinDependency
            stashed_joins&.<< association
          else
            yield association if block_given?
          end
        end
        result
      end

      def build_join_buckets
        buckets = Hash.new { |h, k| h[k] = [] }

        unless left_outer_joins_values.empty?
          stashed_left_joins = []
          left_joins = select_association_list(left_outer_joins_values, stashed_left_joins) do
            raise ArgumentError, "only Hash, Symbol and Array are allowed"
          end

          if joins_values.empty?
            buckets[:association_join] = left_joins
            buckets[:stashed_join] = stashed_left_joins
            return buckets, Arel::Nodes::OuterJoin
          else
            stashed_left_joins.unshift construct_join_dependency(left_joins, Arel::Nodes::OuterJoin)
          end
        end

        joins = joins_values.dup
        if joins.last.is_a?(ActiveRecord::Associations::JoinDependency)
          stashed_eager_load = joins.pop if joins.last.base_klass == klass
        end

        joins.each_with_index do |join, i|
          joins[i] = Arel::Nodes::StringJoin.new(Arel.sql(join.strip)) if join.is_a?(String)
        end

        while joins.first.is_a?(Arel::Nodes::Join)
          join_node = joins.shift
          if !join_node.is_a?(Arel::Nodes::LeadingJoin) && (stashed_eager_load || stashed_left_joins)
            buckets[:join_node] << join_node
          else
            buckets[:leading_join] << join_node
          end
        end

        buckets[:association_join] = select_association_list(joins, buckets[:stashed_join]) do |join|
          if join.is_a?(Arel::Nodes::Join)
            buckets[:join_node] << join
          else
            raise "unknown class: %s" % join.class.name
          end
        end

        buckets[:stashed_join].concat stashed_left_joins if stashed_left_joins
        buckets[:stashed_join] << stashed_eager_load if stashed_eager_load

        return buckets, Arel::Nodes::InnerJoin
      end

      def build_joins(join_sources, aliases = nil)
        return join_sources if joins_values.empty? && left_outer_joins_values.empty?

        buckets, join_type = build_join_buckets

        association_joins = buckets[:association_join]
        stashed_joins     = buckets[:stashed_join]
        leading_joins     = buckets[:leading_join]
        join_nodes        = buckets[:join_node]

        join_sources.concat(leading_joins) unless leading_joins.empty?

        unless association_joins.empty? && stashed_joins.empty?
          alias_tracker = alias_tracker(leading_joins + join_nodes, aliases)
          join_dependency = construct_join_dependency(association_joins, join_type)
          join_sources.concat(join_dependency.join_constraints(stashed_joins, alias_tracker, references_values))
        end

        join_sources.concat(join_nodes) unless join_nodes.empty?
        join_sources
      end

      def build_select(arel)
        if select_values.any?
          arel.project(*arel_columns(select_values))
        elsif klass.ignored_columns.any? || klass.enumerate_columns_in_select_statements
          arel.project(*klass.column_names.map { |field| table[field] })
        else
          arel.project(table[Arel.star])
        end
      end

      def build_with(arel)
        return if with_values.empty?

        with_statements = with_values.map do |with_value|
          raise ArgumentError, "Unsupported argument type: #{with_value} #{with_value.class}" unless with_value.is_a?(Hash)

          build_with_value_from_hash(with_value)
        end

        arel.with(with_statements)
      end

      def build_with_value_from_hash(hash)
        hash.map do |name, value|
          expression =
            case value
            when Arel::Nodes::SqlLiteral then Arel::Nodes::Grouping.new(value)
            when ActiveRecord::Relation then value.arel
            when Arel::SelectManager then value
            else
              raise ArgumentError, "Unsupported argument type: `#{value}` #{value.class}"
            end
          Arel::Nodes::TableAlias.new(expression, name)
        end
      end

      def arel_columns(columns)
        columns.flat_map do |field|
          case field
          when Symbol
            arel_column(field.to_s) do |attr_name|
              connection.quote_table_name(attr_name)
            end
          when String
            arel_column(field, &:itself)
          when Proc
            field.call
          else
            field
          end
        end
      end

      def arel_column(field)
        field = klass.attribute_aliases[field] || field
        from = from_clause.name || from_clause.value

        if klass.columns_hash.key?(field) && (!from || table_name_matches?(from))
          table[field]
        elsif field.match?(/\A\w+\.\w+\z/)
          table, column = field.split(".")
          predicate_builder.resolve_arel_attribute(table, column) do
            lookup_table_klass_from_join_dependencies(table)
          end
        else
          yield field
        end
      end

      def table_name_matches?(from)
        table_name = Regexp.escape(table.name)
        quoted_table_name = Regexp.escape(connection.quote_table_name(table.name))
        /(?:\A|(?<!FROM)\s)(?:\b#{table_name}\b|#{quoted_table_name})(?!\.)/i.match?(from.to_s)
      end

      def reverse_sql_order(order_query)
        if order_query.empty?
          return [table[primary_key].desc] if primary_key
          raise IrreversibleOrderError,
            "Relation has no current order and table has no primary key to be used as default order"
        end

        order_query.flat_map do |o|
          case o
          when Arel::Attribute
            o.desc
          when Arel::Nodes::Ordering
            o.reverse
          when Arel::Nodes::NodeExpression
            o.desc
          when String
            if does_not_support_reverse?(o)
              raise IrreversibleOrderError, "Order #{o.inspect} cannot be reversed automatically"
            end
            o.split(",").map! do |s|
              s.strip!
              s.gsub!(/\sasc\Z/i, " DESC") || s.gsub!(/\sdesc\Z/i, " ASC") || (s << " DESC")
            end
          else
            o
          end
        end
      end

      def does_not_support_reverse?(order)
        # Account for String subclasses like Arel::Nodes::SqlLiteral that
        # override methods like #count.
        order = String.new(order) unless order.instance_of?(String)

        # Uses SQL function with multiple arguments.
        (order.include?(",") && order.split(",").find { |section| section.count("(") != section.count(")") }) ||
          # Uses "nulls first" like construction.
          /\bnulls\s+(?:first|last)\b/i.match?(order)
      end

      def build_order(arel)
        orders = order_values.compact_blank
        arel.order(*orders) unless orders.empty?
      end

      VALID_DIRECTIONS = [:asc, :desc, :ASC, :DESC,
                          "asc", "desc", "ASC", "DESC"].to_set # :nodoc:

      def validate_order_args(args)
        args.each do |arg|
          next unless arg.is_a?(Hash)
          arg.each do |_key, value|
            unless VALID_DIRECTIONS.include?(value)
              raise ArgumentError,
                "Direction \"#{value}\" is invalid. Valid directions are: #{VALID_DIRECTIONS.to_a.inspect}"
            end
          end
        end
      end

      def preprocess_order_args(order_args)
        @klass.disallow_raw_sql!(
          order_args.flat_map { |a| a.is_a?(Hash) ? a.keys : a },
          permit: connection.column_name_with_order_matcher
        )

        validate_order_args(order_args)

        references = column_references(order_args)
        self.references_values |= references unless references.empty?

        # if a symbol is given we prepend the quoted table name
        order_args.map! do |arg|
          case arg
          when Symbol
            order_column(arg.to_s).asc
          when Hash
            arg.map { |field, dir|
              case field
              when Arel::Nodes::SqlLiteral, Arel::Nodes::Node, Arel::Attribute
                field.public_send(dir.downcase)
              else
                order_column(field.to_s).public_send(dir.downcase)
              end
            }
          else
            arg
          end
        end.flatten!
      end

      def sanitize_order_arguments(order_args)
        order_args.map! do |arg|
          klass.sanitize_sql_for_order(arg)
        end
      end

      def column_references(order_args)
        references = order_args.flat_map do |arg|
          case arg
          when String, Symbol
            arg
          when Hash
            arg.keys.map do |key|
              key if key.is_a?(String) || key.is_a?(Symbol)
            end
          end
        end
        references.map! { |arg| arg =~ /^\W?(\w+)\W?\./ && $1 }.compact!
        references
      end

      def order_column(field)
        arel_column(field) do |attr_name|
          if attr_name == "count" && !group_values.empty?
            table[attr_name]
          else
            Arel.sql(connection.quote_table_name(attr_name))
          end
        end
      end

      def build_case_for_value_position(column, values)
        node = Arel::Nodes::Case.new
        values.each.with_index(1) do |value, order|
          node.when(column.eq(value)).then(order)
        end

        Arel::Nodes::Ascending.new(node)
      end

      def resolve_arel_attributes(attrs)
        attrs.flat_map do |attr|
          case attr
          when Arel::Predications
            attr
          when Hash
            attr.flat_map do |table, columns|
              table = table.to_s
              Array(columns).map do |column|
                predicate_builder.resolve_arel_attribute(table, column)
              end
            end
          else
            attr = attr.to_s
            if attr.include?(".")
              table, column = attr.split(".", 2)
              predicate_builder.resolve_arel_attribute(table, column)
            else
              attr
            end
          end
        end
      end

      # Checks to make sure that the arguments are not blank. Note that if some
      # blank-like object were initially passed into the query method, then this
      # method will not raise an error.
      #
      # Example:
      #
      #    Post.references()   # raises an error
      #    Post.references([]) # does not raise an error
      #
      # This particular method should be called with a method_name (__callee__) and the args
      # passed into that method as an input. For example:
      #
      # def references(*args)
      #   check_if_method_has_arguments!(__callee__, args)
      #   ...
      # end
      def check_if_method_has_arguments!(method_name, args, message = nil)
        if args.blank?
          raise ArgumentError, message || "The method .#{method_name}() must contain arguments."
        else
          yield args if block_given?

          args.flatten!
          args.compact_blank!
        end
      end

      def process_select_args(fields)
        fields.flat_map do |field|
          if field.is_a?(Hash)
            transform_select_hash_values(field)
          else
            field
          end
        end
      end

      def transform_select_hash_values(fields)
        fields.flat_map do |key, columns_aliases|
          case columns_aliases
          when Hash
            columns_aliases.map do |column, column_alias|
              arel_column("#{key}.#{column}") do
                predicate_builder.resolve_arel_attribute(key.to_s, column)
              end.as(column_alias.to_s)
            end
          when Array
            columns_aliases.map do |column|
              arel_column("#{key}.#{column}", &:itself)
            end
          when String, Symbol
            arel_column(key.to_s) do
              predicate_builder.resolve_arel_attribute(klass.table_name, key.to_s)
            end.as(columns_aliases.to_s)
          end
        end
      end

      STRUCTURAL_VALUE_METHODS = (
        Relation::VALUE_METHODS -
        [:extending, :where, :having, :unscope, :references, :annotate, :optimizer_hints]
      ).freeze # :nodoc:

      def structurally_incompatible_values_for(other)
        values = other.values
        STRUCTURAL_VALUE_METHODS.reject do |method|
          v1, v2 = @values[method], values[method]
          if v1.is_a?(Array)
            next true unless v2.is_a?(Array)
            v1 = v1.uniq
            v2 = v2.uniq
          end
          v1 == v2
        end
      end
  end
end