
    eager_autoload do
      autoload :Error
      autoload :Handlers
      autoload :HTML
      autoload :Inline
      autoload :Text
      @virtual_path      = virtual_path

      @variable = if @virtual_path
        base = @virtual_path[-1] == "/" ? "" : File.basename(@virtual_path)
        base =~ /\A_?(.*?)(?:\.\w+)*\z/
        $1.to_sym
      end
