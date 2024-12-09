  def get(key)
    if /latency/.match?(key)
      sleep 3
    else
      super
    end
  end