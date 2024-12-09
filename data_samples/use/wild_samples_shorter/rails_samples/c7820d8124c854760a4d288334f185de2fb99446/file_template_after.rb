    end

    def source
      ::File.binread @filename
    end

    def refresh(_)
      self