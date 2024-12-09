        when Proc
          strategy.call(request.raw_post)
        when :xml_simple, :xml_node
          data = request.deep_munge(Hash.from_xml(request.body.read) || {})
          data.with_indifferent_access
        when :yaml
          YAML.load(request.raw_post)
        when :json
          data = request.deep_munge ActiveSupport::JSON.decode(request.body)
          data = {:_json => data} unless data.is_a?(Hash)
          data.with_indifferent_access
        else
          false