      end

      def sanitize_as_sql_comment(value) # :nodoc:
        # Sanitize a string to appear within a SQL comment
        # For compatibility, this also surrounding "/*+", "/*", and "*/"
        # charcacters, possibly with single surrounding space.
        # Then follows that by replacing any internal "*/" or "/ *" with
        # "* /" or "/ *"
        comment = value.to_s.dup
        comment.gsub!(%r{\A\s*/\*\+?\s?|\s?\*/\s*\Z}, "")
        comment.gsub!("*/", "* /")
        comment.gsub!("/*", "/ *")
        comment
      end

      def column_name_matcher # :nodoc:
        COLUMN_NAME