          else
            [Mime[:html]]
          end

          v = v.select do |format|
            format.symbol || format.ref == "*/*"
          end

          set_header k, v
        end
      end
