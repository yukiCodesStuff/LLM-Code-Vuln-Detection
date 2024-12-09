      # +false+ to change that):
      #   number_to_human(12345012345, significant_digits: 6)       # => "12.345 Billion"
      #   number_to_human(500000000, precision: 5)                  # => "500 Million"
      #
      # ==== Custom Unit Quantifiers
      #
      # You can also use your own custom unit quantifiers:
      #  number_to_human(500000, units: {unit: "ml", thousand: "lt"})  # => "500 lt"
      #
      # If in your I18n locale you have:
      #   distance:
      #     centi:
      #       one: "centimeter"
      #       other: "centimeters"
      #     unit:
      #       one: "meter"
      #       other: "meters"
      #     thousand:
      #       one: "kilometer"
      #       other: "kilometers"
      #     billion: "gazillion-distance"
      #
      # Then you could do:
      #
      #  number_to_human(543934, units: :distance)              # => "544 kilometers"
      #  number_to_human(54393498, units: :distance)            # => "54400 kilometers"
      #  number_to_human(54393498000, units: :distance)         # => "54.4 gazillion-distance"
      #  number_to_human(343, units: :distance, precision: 1)   # => "300 meters"
      #  number_to_human(1, units: :distance)                   # => "1 meter"
      #  number_to_human(0.34, units: :distance)                # => "34 centimeters"
      #
      def number_to_human(number, options = {})
        delegate_number_helper_method(:number_to_human, number, options)
      end

      private

      def delegate_number_helper_method(method, number, options)
        return unless number
        options = escape_unsafe_delimiters_and_separators(options.symbolize_keys)

        wrap_with_output_safety_handling(number, options.delete(:raise)) {
          ActiveSupport::NumberHelper.public_send(method, number, options)
        }