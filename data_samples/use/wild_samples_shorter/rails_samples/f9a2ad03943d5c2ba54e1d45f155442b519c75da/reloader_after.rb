      prepare!
    end

    def self.run!(reset: false) # :nodoc:
      if check!
        super
      else
        Null