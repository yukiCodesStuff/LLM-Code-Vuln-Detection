      end

      def sanitize_as_sql_comment(value) # :nodoc:
        value.to_s.gsub(%r{ (/ (?: | \g<1>) \*) \+? \s* | \s* (\* (?: | \g<2>) /) }x, "")
      end

      def column_name_matcher # :nodoc:
        COLUMN_NAME