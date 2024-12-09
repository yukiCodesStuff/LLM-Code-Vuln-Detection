        "WHERE #{quoted_primary_key} IN (SELECT #{quoted_primary_key} FROM #{quoted_table_name} #{where_sql})"
      end

      # Sanitizes the given LIMIT parameter in order to prevent SQL injection.
      #
      # +limit+ may be anything that can evaluate to a string via #to_s. It
      # should look like an integer, or a comma-delimited list of integers.
      #
      # Returns the sanitized limit parameter, either as an integer, or as a
      # string which contains a comma-delimited list of integers.
      def sanitize_limit(limit)
        if limit.to_s =~ /,/
          Arel.sql limit.to_s.split(',').map{ |i| Integer(i) }.join(',')
        else
          Integer(limit)
        end
      end

      protected
        # Returns an array of record hashes with the column names as keys and
        # column values as values.
        def select(sql, name = nil, binds = [])
          update_sql(sql, name)
        end

        # Send a rollback message to all records after they have been rolled back. If rollback
        # is false, only rollback records since the last save point.
        def rollback_transaction_records(rollback) #:nodoc
          if rollback