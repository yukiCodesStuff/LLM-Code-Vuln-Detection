        email_address_obfuscated = email_address.dup
        email_address_obfuscated.gsub!(/@/, html_options.delete("replace_at")) if html_options.key?("replace_at")
        email_address_obfuscated.gsub!(/\./, html_options.delete("replace_dot")) if html_options.key?("replace_dot")
        case encode
        when "javascript"
          string = ''
          html   = content_tag("a", name || email_address_obfuscated.html_safe, html_options.merge("href" => "mailto:#{email_address}#{extras}".html_safe))
          html   = escape_javascript(html)
          "document.write('#{html}');".each_byte do |c|
            string << sprintf("%%%x", c)
          end
          "<script type=\"#{Mime::JS}\">eval(decodeURIComponent('#{string}'))</script>".html_safe
        when "hex"
          email_address_encoded = email_address_obfuscated.unpack('C*').map {|c|
            sprintf("&#%d;", c)