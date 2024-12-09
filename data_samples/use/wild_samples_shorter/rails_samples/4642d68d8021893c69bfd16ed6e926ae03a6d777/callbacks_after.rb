    # halt the entire callback chain and display a deprecation message.
    # If false, callback chains will only be halted by calling +throw :abort+.
    # Defaults to +true+.
    mattr_accessor(:halt_and_display_warning_on_return_false, instance_writer: false) { true }

    # Runs the callbacks for the given event.
    #
    # Calls the before and around callbacks in the order they were set, yields
        options = names.extract_options!

        names.each do |name|
          class_attribute "_#{name}_callbacks", instance_writer: false
          set_callbacks name, CallbackChain.new(name, options)

          module_eval <<-RUBY, __FILE__, __LINE__ + 1
            def _run_#{name}_callbacks(&block)