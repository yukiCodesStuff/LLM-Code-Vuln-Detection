      )
    end

    attr_accessor :depth
    self.depth = 100

    delegate :parse, :to => :backend

    def backend
      current_thread_backend || @backend