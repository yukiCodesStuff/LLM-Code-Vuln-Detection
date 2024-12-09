    def process(action, *args)
      @_action_name = action.to_s

      unless action_name = _find_action_name(@_action_name)
        raise ActionNotFound, "The action '#{action}' could not be found for #{self.class.name}"
      end

      @_response_body = nil
    # ==== Returns
    # * <tt>TrueClass</tt>, <tt>FalseClass</tt>
    def available_action?(action_name)
      _find_action_name(action_name).present?
    end

    private

        action_missing(@_action_name, *args)
      end

      # Takes an action name and returns the name of the method that will
      # handle the action.
      #
      # It checks if the action name is valid and returns false otherwise.
      #
      # See method_for_action for more information.
      #
      # ==== Parameters
      # * <tt>action_name</tt> - An action name to find a method name for
      #
      # ==== Returns
      # * <tt>string</tt> - The name of the method that handles the action
      # * false           - No valid method name could be found. Raise ActionNotFound.
      def _find_action_name(action_name)
        _valid_action_name?(action_name) && method_for_action(action_name)
      end

      # Takes an action name and returns the name of the method that will
      # handle the action. In normal cases, this method returns the same
      # name as it receives. By default, if #method_for_action receives
      # a name that is not an action, it will look for an #action_missing
      #
      # ==== Returns
      # * <tt>string</tt> - The name of the method that handles the action
      # * <tt>nil</tt>    - No method name could be found.
      def method_for_action(action_name)
        if action_method?(action_name)
          action_name
        elsif respond_to?(:action_missing, true)
          "_handle_action_missing"
        end
      end

      # Checks if the action name is valid and returns false otherwise.
      def _valid_action_name?(action_name)
        action_name.to_s !~ Regexp.new(File::SEPARATOR)
      end
  end
end