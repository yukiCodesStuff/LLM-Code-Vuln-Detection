            block.arity <= 1 ? capture(name, &block) : capture(name, options, html_options, &block)
          else
            name
          end
        else
          link_to(name, options, html_options)
        end
      end

      # Creates a link tag of the given +name+ using a URL created by the set of
      # +options+ if +condition+ is true, in which case only the name is
      # returned. To specialize the default behavior, you can pass a block that
      # accepts the name or the full argument list for +link_to_unless+ (see the examples
      # in +link_to_unless+).
      #
      # ==== Examples
      #   <%= link_to_if(@current_user.nil?, "Login", { :controller => "sessions", :action => "new" }) %>
      #   # If the user isn't logged in...
      #   # => <a href="/sessions/new/">Login</a>
      #
      #   <%=
      #      link_to_if(@current_user.nil?, "Login", { :controller => "sessions", :action => "new" }) do
      #        link_to(@current_user.login, { :controller => "accounts", :action => "show", :id => @current_user })
      #      end
      #   %>
      #   # If the user isn't logged in...
      #   # => <a href="/sessions/new/">Login</a>
      #   # If they are logged in...
      #   # => <a href="/accounts/show/3">my_username</a>
      def link_to_if(condition, name, options = {}, html_options = {}, &block)
        link_to_unless !condition, name, options, html_options, &block
      end

      # Creates a mailto link tag to the specified +email_address+, which is
      # also used as the name of the link unless +name+ is specified. Additional
      # HTML attributes for the link can be passed in +html_options+.
      #
      # +mail_to+ has several methods for hindering email harvesters and customizing
      # the email itself by passing special keys to +html_options+.
      #
      # ==== Options
      # * <tt>:encode</tt> - This key will accept the strings "javascript" or "hex".
      #   Passing "javascript" will dynamically create and encode the mailto link then
      #   eval it into the DOM of the page. This method will not show the link on
      #   the page if the user has JavaScript disabled. Passing "hex" will hex
      #   encode the +email_address+ before outputting the mailto link.
      # * <tt>:replace_at</tt> - When the link +name+ isn't provided, the
      #   +email_address+ is used for the link label. You can use this option to
      #   obfuscate the +email_address+ by substituting the @ sign with the string
      #   given as the value.
      # * <tt>:replace_dot</tt> - When the link +name+ isn't provided, the
      #   +email_address+ is used for the link label. You can use this option to
      #   obfuscate the +email_address+ by substituting the . in the email with the
      #   string given as the value.
      # * <tt>:subject</tt> - Preset the subject line of the email.
      # * <tt>:body</tt> - Preset the body of the email.
      # * <tt>:cc</tt> - Carbon Copy addition recipients on the email.
      # * <tt>:bcc</tt> - Blind Carbon Copy additional recipients on the email.
      #
      # ==== Examples
      #   mail_to "me@domain.com"
      #   # => <a href="mailto:me@domain.com">me@domain.com</a>
      #
      #   mail_to "me@domain.com", "My email", :encode => "javascript"
      #   # => <script type="text/javascript">eval(decodeURIComponent('%64%6f%63...%27%29%3b'))</script>
      #
      #   mail_to "me@domain.com", "My email", :encode => "hex"
      #   # => <a href="mailto:%6d%65@%64%6f%6d%61%69%6e.%63%6f%6d">My email</a>
      #
      #   mail_to "me@domain.com", nil, :replace_at => "_at_", :replace_dot => "_dot_", :class => "email"
      #   # => <a href="mailto:me@domain.com" class="email">me_at_domain_dot_com</a>
      #
      #   mail_to "me@domain.com", "My email", :cc => "ccaddress@domain.com",
      #            :subject => "This is an example email"
      #   # => <a href="mailto:me@domain.com?cc=ccaddress@domain.com&subject=This%20is%20an%20example%20email">My email</a>
      def mail_to(email_address, name = nil, html_options = {})
        email_address = ERB::Util.html_escape(email_address)

        html_options = html_options.stringify_keys
        encode = html_options.delete("encode").to_s

        extras = %w{ cc bcc body subject }.map { |item|
          option = html_options.delete(item) || next
          "#{item}=#{Rack::Utils.escape(option).gsub("+", "%20")}"
        }.compact
        extras = extras.empty? ? '' : '?' + ERB::Util.html_escape(extras.join('&'))

        email_address_obfuscated = email_address.dup
        email_address_obfuscated.gsub!(/@/, html_options.delete("replace_at")) if html_options.key?("replace_at")
        email_address_obfuscated.gsub!(/\./, html_options.delete("replace_dot")) if html_options.key?("replace_dot")

        case encode
        when "javascript"
          string =
            "document.write('#{content_tag("a", name || email_address_obfuscated.html_safe, html_options.merge("href" => "mailto:#{email_address}#{extras}".html_safe))}');".unpack('C*').map { |c|
            sprintf("%%%x", c)
          }.join
          "<script type=\"#{Mime::JS}\">eval(decodeURIComponent('#{string}'))</script>".html_safe
        when "hex"
          email_address_encoded = email_address_obfuscated.unpack('C*').map {|c|
            sprintf("&#%d;", c)
          }.join

          string = 'mailto:'.unpack('C*').map { |c|
            sprintf("&#%d;", c)
          }.join + email_address.unpack('C*').map { |c|
            char = c.chr
            char =~ /\w/ ? sprintf("%%%x", c) : char
          }.join

          content_tag "a", name || email_address_encoded.html_safe, html_options.merge("href" => "#{string}#{extras}".html_safe)
        else
          content_tag "a", name || email_address_obfuscated.html_safe, html_options.merge("href" => "mailto:#{email_address}#{extras}".html_safe)
        end
      end

      # True if the current request URI was generated by the given +options+.
      #
      # ==== Examples
      # Let's say we're in the <tt>/shop/checkout?order=desc</tt> action.
      #
      #   current_page?(:action => 'process')
      #   # => false
      #
      #   current_page?(:controller => 'shop', :action => 'checkout')
      #   # => true
      #
      #   current_page?(:controller => 'shop', :action => 'checkout', :order => 'asc')
      #   # => false
      #
      #   current_page?(:action => 'checkout')
      #   # => true
      #
      #   current_page?(:controller => 'library', :action => 'checkout')
      #   # => false
      #
      # Let's say we're in the <tt>/shop/checkout?order=desc&page=1</tt> action.
      #
      #   current_page?(:action => 'process')
      #   # => false
      #
      #   current_page?(:controller => 'shop', :action => 'checkout')
      #   # => true
      #
      #   current_page?(:controller => 'shop', :action => 'checkout', :order => 'desc', :page=>'1')
      #   # => true
      #
      #   current_page?(:controller => 'shop', :action => 'checkout', :order => 'desc', :page=>'2')
      #   # => false
      #
      #   current_page?(:controller => 'shop', :action => 'checkout', :order => 'desc')
      #   # => false
      #
      #   current_page?(:action => 'checkout')
      #   # => true
      #
      #   current_page?(:controller => 'library', :action => 'checkout')
      #   # => false
      def current_page?(options)
        unless request
          raise "You cannot use helpers that need to determine the current " \
                "page unless your view context provides a Request object " \
                "in a #request method"
        end

        url_string = url_for(options)

        # We ignore any extra parameters in the request_uri if the
        # submitted url doesn't have any either.  This lets the function
        # work with things like ?order=asc
        if url_string.index("?")
          request_uri = request.fullpath
        else
          request_uri = request.path
        end

        if url_string =~ /^\w+:\/\//
          url_string == "#{request.protocol}#{request.host_with_port}#{request_uri}"
        else
          url_string == request_uri
        end
      end

      private
        def convert_options_to_data_attributes(options, html_options)
          if html_options.nil?
            link_to_remote_options?(options) ? {'data-remote' => 'true'} : {}
          else
            html_options = html_options.stringify_keys
            html_options['data-remote'] = 'true' if link_to_remote_options?(options) || link_to_remote_options?(html_options)

            disable_with = html_options.delete("disable_with")
            confirm = html_options.delete('confirm')
            method  = html_options.delete('method')

            html_options["data-disable-with"] = disable_with if disable_with
            html_options["data-confirm"] = confirm if confirm
            add_method_to_attributes!(html_options, method)   if method

            html_options
          end
        end

        def link_to_remote_options?(options)
          options.is_a?(Hash) && options.key?('remote') && options.delete('remote')
        end

        def add_method_to_attributes!(html_options, method)
          html_options["rel"] = "nofollow" if method.to_s.downcase != "get"
          html_options["data-method"] = method
        end

        def options_for_javascript(options)
          if options.empty?
            '{}'
          else
            "{#{options.keys.map { |k| "#{k}:#{options[k]}" }.sort.join(', ')}}"
          end
        end

        def array_or_string_for_javascript(option)
          if option.kind_of?(Array)
            "['#{option.join('\',\'')}']"
          elsif !option.nil?
            "'#{option}'"
          end
        end

        # Processes the +html_options+ hash, converting the boolean
        # attributes from true/false form into the form required by
        # HTML/XHTML.  (An attribute is considered to be boolean if
        # its name is listed in the given +bool_attrs+ array.)
        #
        # More specifically, for each boolean attribute in +html_options+
        # given as:
        #
        #   "attr" => bool_value
        #
        # if the associated +bool_value+ evaluates to true, it is
        # replaced with the attribute's name; otherwise the attribute is
        # removed from the +html_options+ hash.  (See the XHTML 1.0 spec,
        # section 4.5 "Attribute Minimization" for more:
        # http://www.w3.org/TR/xhtml1/#h-4.5)
        #
        # Returns the updated +html_options+ hash, which is also modified
        # in place.
        #
        # Example:
        #
        #   convert_boolean_attributes!( html_options,
        #                                %w( checked disabled readonly ) )
        def convert_boolean_attributes!(html_options, bool_attrs)
          bool_attrs.each { |x| html_options[x] = x if html_options.delete(x) }
          html_options
        end
    end
  end
end