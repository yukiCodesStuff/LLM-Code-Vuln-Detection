      style = style.to_s.gsub(/url\s*\(\s*[^\s)]+?\s*\)\s*/, ' ')

      # gauntlet
      if style !~ /\A([:,;#%.\sa-zA-Z0-9!]|\w-\w|\'[\s\w]+\'|\"[\s\w]+\"|\([\d,\s]+\))*\z/ ||
          style !~ /\A(\s*[-\w]+\s*:\s*[^:;]*(;|$)\s*)*\z/
        return ''
      end

      clean = []
        elsif shorthand_css_properties.include?(prop.split('-')[0].downcase)
          unless val.split().any? do |keyword|
            !allowed_css_keywords.include?(keyword) &&
              keyword !~ /\A(#[0-9a-f]+|rgb\(\d+%?,\d*%?,?\d*%?\)?|\d{0,2}\.?\d{0,2}(cm|em|ex|in|mm|pc|pt|px|%|,|\))?)\z/
          end
            clean << prop + ': ' + val + ';'
          end
        end