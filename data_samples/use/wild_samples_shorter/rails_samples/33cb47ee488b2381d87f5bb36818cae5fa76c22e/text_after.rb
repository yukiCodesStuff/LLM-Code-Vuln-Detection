      end

      def formats
        [@type.respond_to?(:ref) ? @type.ref : @type.to_s]
      end
    end
  end
end