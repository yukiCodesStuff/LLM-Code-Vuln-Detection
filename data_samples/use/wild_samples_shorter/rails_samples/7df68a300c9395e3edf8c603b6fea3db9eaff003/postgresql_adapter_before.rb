        end

        def exec_no_cache(sql, name, binds)
          log(sql, name, binds) { @connection.async_exec(sql) }
        end

        def exec_cache(sql, name, binds)
          stmt_key = prepare_statement(sql)