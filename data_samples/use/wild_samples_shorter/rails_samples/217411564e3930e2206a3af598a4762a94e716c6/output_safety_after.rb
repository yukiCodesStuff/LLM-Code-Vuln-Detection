    end
    alias << concat

    def bytesplice(*args, value)
      super(*args, implicit_html_escape_interpolated_argument(value))
    end

    def insert(index, value)
      super(index, implicit_html_escape_interpolated_argument(value))
    end
