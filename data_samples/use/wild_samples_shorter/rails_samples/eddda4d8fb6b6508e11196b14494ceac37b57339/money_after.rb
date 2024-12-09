
            value = value.sub(/^\((.+)\)$/, '-\1') # (4)
            case value
            when /^-?\D*+[\d,]+\.\d{2}$/  # (1)
              value.gsub!(/[^-\d.]/, "")
            when /^-?\D*+[\d.]+,\d{2}$/  # (2)
              value.gsub!(/[^-\d,]/, "").sub!(/,/, ".")
            end

            super(value)