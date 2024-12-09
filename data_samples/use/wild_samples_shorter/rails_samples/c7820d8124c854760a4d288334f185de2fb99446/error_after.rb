      def line_number
        @line_number ||=
          if file_name
            regexp = /#{Regexp.escape ::File.basename(file_name)}:(\d+)/
            $1 if message =~ regexp || backtrace.find { |line| line =~ regexp }
          end
      end
