        "\n"    => '\n',
        "\r"    => '\n',
        '"'     => '\\"',
        "'"     => "\\'"
      }

      JS_ESCAPE_MAP[(+"\342\200\250").force_encoding(Encoding::UTF_8).encode!] = "&#x2028;"
      JS_ESCAPE_MAP[(+"\342\200\251").force_encoding(Encoding::UTF_8).encode!] = "&#x2029;"
        if javascript.empty?
          result = ""
        else
          result = javascript.gsub(/(\\|<\/|\r\n|\342\200\250|\342\200\251|[\n\r"'])/u, JS_ESCAPE_MAP)
        end
        javascript.html_safe? ? result.html_safe : result
      end
