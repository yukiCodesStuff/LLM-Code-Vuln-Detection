        end
      end
    end

    initializer "active_record.use_yaml_unsafe_load" do |app|
      config.after_initialize do
        unless app.config.active_record.use_yaml_unsafe_load.nil?
          ActiveRecord.use_yaml_unsafe_load =
            app.config.active_record.use_yaml_unsafe_load
        end
      end
    end

    initializer "active_record.yaml_column_permitted_classes" do |app|
      config.after_initialize do
        unless app.config.active_record.yaml_column_permitted_classes.nil?
          ActiveRecord.yaml_column_permitted_classes =
            app.config.active_record.yaml_column_permitted_classes
        end
      end
    end
  end
end