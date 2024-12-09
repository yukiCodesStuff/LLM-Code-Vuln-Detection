
    def relative_path
      @secret = "in the sauce"
      render file: "../../fixtures/test/render_file_with_ivar"
    end

    def relative_path_with_dot
      @secret = "in the sauce"
      render file: "../../fixtures/test/dot.directory/render_file_with_ivar"
    end

    def pathname
      @secret = "in the sauce"