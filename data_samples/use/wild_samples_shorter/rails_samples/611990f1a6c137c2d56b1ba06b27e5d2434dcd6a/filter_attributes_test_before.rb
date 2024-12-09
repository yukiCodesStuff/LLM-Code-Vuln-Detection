  setup do
    @previous_filter_attributes = ActiveRecord::Base.filter_attributes
    ActiveRecord::Base.filter_attributes = [:name]
  end

  teardown do
    ActiveRecord::Base.filter_attributes = @previous_filter_attributes