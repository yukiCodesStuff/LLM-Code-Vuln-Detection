    end
    alias << concat

    def insert(index, value)
      super(index, implicit_html_escape_interpolated_argument(value))
    end
