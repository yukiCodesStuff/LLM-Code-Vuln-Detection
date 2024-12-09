          else
            value = escape ? ERB::Util.unwrapped_html_escape(value) : value
          end
          %(#{key}="#{value}")
        end

        private
          def prefix_tag_option(prefix, key, value, escape)