        end

        def if_none_match_etags
          if_none_match ? if_none_match.split(",").each(&:strip!) : []
        end

        def not_modified?(modified_at)
          if_modified_since && modified_at && if_modified_since >= modified_at