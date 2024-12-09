      prepare!
    end

    def self.run! # :nodoc:
      if check!
        super
      else
        Null