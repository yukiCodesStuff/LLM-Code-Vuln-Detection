      end
      alias :find_template :find

      def find_file(name, prefixes = [], partial = false, keys = [], options = {})
        @view_paths.find_file(*args_for_lookup(name, prefixes, partial, keys, options))
      end

      def find_all(name, prefixes = [], partial = false, keys = [], options = {})
        @view_paths.find_all(*args_for_lookup(name, prefixes, partial, keys, options))
      end
