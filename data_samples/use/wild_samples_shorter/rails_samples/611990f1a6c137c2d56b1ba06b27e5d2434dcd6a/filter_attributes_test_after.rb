  setup do
    @previous_filter_attributes = ActiveRecord::Base.filter_attributes
    ActiveRecord::Base.filter_attributes = [:name]
    ActiveRecord.use_yaml_unsafe_load = true
  end

  teardown do
    ActiveRecord::Base.filter_attributes = @previous_filter_attributes