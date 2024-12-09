    # after the work has been performed.
    #
    # Where possible, prefer +wrap+.
    def self.run!(reset: false)
      if reset
        lost_instance = IsolatedExecutionState.delete(active_key)
        lost_instance&.complete!
      else
        return Null if active?
      end

      new.tap do |instance|
        success = nil
        begin
          instance.run!
          success = true
        ensure
          instance.complete! unless success
        end
      end
    end

      end
    end

    def self.error_reporter
      @error_reporter ||= ActiveSupport::ErrorReporter.new
    end

    def self.active_key # :nodoc:
      @active_key ||= :"active_execution_wrapper_#{object_id}"
    end

    def self.active? # :nodoc:
      IsolatedExecutionState.key?(active_key)
    end

    def run! # :nodoc:
      IsolatedExecutionState[self.class.active_key] = self
      run
    end

    def run # :nodoc:
    def complete!
      complete
    ensure
      IsolatedExecutionState.delete(self.class.active_key)
    end

    def complete # :nodoc:
      run_callbacks(:complete)