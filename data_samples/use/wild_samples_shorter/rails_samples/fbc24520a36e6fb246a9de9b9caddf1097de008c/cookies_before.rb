    class CookieJar # :nodoc:
      include Enumerable, ChainedCookieJars

      # This regular expression is used to split the levels of a domain.
      # The top level domain can be any string without a period or
      # **.**, ***.** style TLDs like co.uk or com.au
      #
      # www.example.co.uk gives:
      # $& => example.co.uk
      #
      # example.com gives:
      # $& => example.com
      #
      # lots.of.subdomains.example.local gives:
      # $& => example.local
      DOMAIN_REGEXP = /[^.]*\.([^.]*|..\...|...\...)$/

      def self.build(req, cookies)
        jar = new(req)
        jar.update(cookies)
        jar
          end

          if options[:domain] == :all || options[:domain] == "all"
            # If there is a provided tld length then we use it otherwise default domain regexp.
            domain_regexp = options[:tld_length] ? /([^.]+\.?){#{options[:tld_length]}}$/ : DOMAIN_REGEXP

            # If host is not ip and matches domain regexp.
            # (ip confirms to domain regexp so we explicitly check for ip)
            options[:domain] = if !request.host.match?(/^[\d.]+$/) && (request.host =~ domain_regexp)
              ".#{$&}"
            end
          elsif options[:domain].is_a? Array
            # If host matches one of the supplied domains.
            options[:domain] = options[:domain].find do |domain|