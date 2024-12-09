      end
      alias :find_template :find

      alias :find_file :find
      deprecate :find_file

      def find_all(name, prefixes = [], partial = false, keys = [], options = {})
        @view_paths.find_all(*args_for_lookup(name, prefixes, partial, keys, options))
      end