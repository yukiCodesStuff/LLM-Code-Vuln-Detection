      )
    end

    delegate :parse, :to => :backend

    def backend
      current_thread_backend || @backend