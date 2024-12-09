  def get(key)
    if /latency/.match?(key)
      sleep 3
      super
    else
      super
    end
  end