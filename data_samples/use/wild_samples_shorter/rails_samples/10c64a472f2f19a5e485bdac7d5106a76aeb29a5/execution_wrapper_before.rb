    # after the work has been performed.
    #
    # Where possible, prefer +wrap+.
    def self.run!
      if active?
        Null
      else
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
    end

    class << self # :nodoc:
      attr_accessor :active
    end

    def self.error_reporter
      @error_reporter ||= ActiveSupport::ErrorReporter.new
    end

    def self.inherited(other) # :nodoc:
      super
      other.active = Concurrent::Hash.new
    end

    self.active = Concurrent::Hash.new

    def self.active? # :nodoc:
      @active[IsolatedExecutionState.unique_id]
    end

    def run! # :nodoc:
      self.class.active[IsolatedExecutionState.unique_id] = true
      run
    end

    def run # :nodoc:
    def complete!
      complete
    ensure
      self.class.active.delete(IsolatedExecutionState.unique_id)
    end

    def complete # :nodoc:
      run_callbacks(:complete)