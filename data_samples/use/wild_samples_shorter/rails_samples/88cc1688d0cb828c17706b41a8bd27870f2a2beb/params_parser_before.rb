        when Proc
          strategy.call(request.raw_post)
        when :xml_simple, :xml_node
          data = Hash.from_xml(request.raw_post) || {}
          data.with_indifferent_access
        when :yaml
          YAML.load(request.raw_post)
        when :json
          data = ActiveSupport::JSON.decode(request.raw_post)
          data = {:_json => data} unless data.is_a?(Hash)
          data.with_indifferent_access
        else
          false