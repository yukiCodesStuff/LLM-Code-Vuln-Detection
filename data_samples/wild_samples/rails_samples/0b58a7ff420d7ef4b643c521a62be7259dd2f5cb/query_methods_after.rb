require 'active_support/core_ext/array/wrap'
require 'active_support/core_ext/object/blank'

module ActiveRecord
  module QueryMethods
    extend ActiveSupport::Concern

    attr_accessor :includes_values, :eager_load_values, :preload_values,
                  :select_values, :group_values, :order_values, :joins_values,
                  :where_values, :having_values, :bind_values,
                  :limit_value, :offset_value, :lock_value, :readonly_value, :create_with_value, :from_value

    def includes(*args)
      args.reject! {|a| a.blank? }

      return self if args.empty?

      relation = clone
      relation.includes_values = (relation.includes_values + args).flatten.uniq
      relation
    end

    def eager_load(*args)
      return self if args.blank?

      relation = clone
      relation.eager_load_values += args
      relation
    end

    def preload(*args)
      return self if args.blank?

      relation = clone
      relation.preload_values += args
      relation
    end

    def select(value = Proc.new)
      if block_given?
        to_a.select {|*block_args| value.call(*block_args) }
      else
        relation = clone
        relation.select_values += Array.wrap(value)
        relation
      end
    end

    def group(*args)
      return self if args.blank?

      relation = clone
      relation.group_values += args.flatten
      relation
    end

    def order(*args)
      return self if args.blank?

      relation = clone
      relation.order_values += args.flatten
      relation
    end

    def joins(*args)
      return self if args.compact.blank?

      relation = clone

      args.flatten!
      relation.joins_values += args

      relation
    end

    def bind(value)
      relation = clone
      relation.bind_values += [value]
      relation
    end

    def where(opts, *rest)
      return self if opts.blank?

      relation = clone
      relation.where_values += build_where(opts, rest)
      relation
    end

    def having(*args)
      return self if args.blank?

      relation = clone
      relation.having_values += build_where(*args)
      relation
    end

    def limit(value)
      relation = clone
      relation.limit_value = value
      relation
    end

    def offset(value)
      relation = clone
      relation.offset_value = value
      relation
    end

    def lock(locks = true)
      relation = clone

      case locks
      when String, TrueClass, NilClass
        relation.lock_value = locks || true
      else
        relation.lock_value = false
      end

      relation
    end

    def readonly(value = true)
      relation = clone
      relation.readonly_value = value
      relation
    end

    def create_with(value)
      relation = clone
      relation.create_with_value = value && (@create_with_value || {}).merge(value)
      relation
    end

    def from(value)
      relation = clone
      relation.from_value = value
      relation
    end

    def extending(*modules)
      modules << Module.new(&Proc.new) if block_given?

      return self if modules.empty?

      relation = clone
      relation.send(:apply_modules, modules.flatten)
      relation
    end

    def reverse_order
      order_clause = arel.order_clauses

      order = order_clause.empty? ?
        "#{table_name}.#{primary_key} DESC" :
        reverse_sql_order(order_clause).join(', ')

      except(:order).order(Arel.sql(order))
    end

    def arel
      @arel ||= build_arel
    end

    def build_arel
      arel = table.from table

      build_joins(arel, @joins_values) unless @joins_values.empty?

      collapse_wheres(arel, (@where_values - ['']).uniq)

      arel.having(*@having_values.uniq.reject{|h| h.blank?}) unless @having_values.empty?

      arel.take(connection.sanitize_limit(@limit_value)) if @limit_value
      arel.skip(@offset_value) if @offset_value

      arel.group(*@group_values.uniq.reject{|g| g.blank?}) unless @group_values.empty?

      arel.order(*@order_values.uniq.reject{|o| o.blank?}) unless @order_values.empty?

      build_select(arel, @select_values.uniq)

      arel.from(@from_value) if @from_value
      arel.lock(@lock_value) if @lock_value

      arel
    end

    private

    def custom_join_ast(table, joins)
      joins = joins.reject { |join| join.blank? }

      return [] if joins.empty?

      @implicit_readonly = true

      joins.map do |join|
        case join
        when Array
          join = Arel.sql(join.join(' ')) if array_of_strings?(join)
        when String
          join = Arel.sql(join)
        end
        table.create_string_join(join)
      end
    end

    def collapse_wheres(arel, wheres)
      equalities = wheres.grep(Arel::Nodes::Equality)

      groups = equalities.group_by do |equality|
        equality.left
      end

      groups.each do |_, eqls|
        test = eqls.inject(eqls.shift) do |memo, expr|
          memo.or(expr)
        end
        arel.where(test)
      end

      (wheres - equalities).each do |where|
        where = Arel.sql(where) if String === where
        arel.where(Arel::Nodes::Grouping.new(where))
      end
    end

    def build_where(opts, other = [])
      case opts
      when String, Array
        [@klass.send(:sanitize_sql, other.empty? ? opts : ([opts] + other))]
      when Hash
        attributes = @klass.send(:expand_hash_conditions_for_aggregates, opts)
        PredicateBuilder.build_from_hash(table.engine, attributes, table)
      else
        [opts]
      end
    end

    def build_joins(manager, joins)
      buckets = joins.group_by do |join|
        case join
        when String
          'string_join'
        when Hash, Symbol, Array
          'association_join'
        when ActiveRecord::Associations::ClassMethods::JoinDependency::JoinAssociation
          'stashed_join'
        when Arel::Nodes::Join
          'join_node'
        else
          raise 'unknown class: %s' % join.class.name
        end
      end

      association_joins         = buckets['association_join'] || []
      stashed_association_joins = buckets['stashed_join'] || []
      join_nodes                = buckets['join_node'] || []
      string_joins              = (buckets['string_join'] || []).map { |x|
        x.strip
      }.uniq

      join_list = custom_join_ast(manager, string_joins)

      join_dependency = ActiveRecord::Associations::ClassMethods::JoinDependency.new(
        @klass,
        association_joins,
        join_list
      )

      join_nodes.each do |join|
        join_dependency.table_aliases[join.left.name.downcase] = 1
      end

      join_dependency.graft(*stashed_association_joins)

      @implicit_readonly = true unless association_joins.empty? && stashed_association_joins.empty?

      # FIXME: refactor this to build an AST
      join_dependency.join_associations.each do |association|
        association.join_to(manager)
      end

      manager.join_sources.concat join_nodes.uniq
      manager.join_sources.concat join_list

      manager
    end

    def build_select(arel, selects)
      unless selects.empty?
        @implicit_readonly = false
        arel.project(*selects)
      else
        arel.project(Arel.sql(@klass.quoted_table_name + '.*'))
      end
    end

    def apply_modules(modules)
      unless modules.empty?
        @extensions += modules
        modules.each {|extension| extend(extension) }
      end
    end

    def reverse_sql_order(order_query)
      order_query.join(', ').split(',').collect do |s|
        s.gsub!(/\sasc\Z/i, ' DESC') || s.gsub!(/\sdesc\Z/i, ' ASC') || s.concat(' DESC')
      end
    end

    def array_of_strings?(o)
      o.is_a?(Array) && o.all?{|obj| obj.is_a?(String)}
    end

  end
end