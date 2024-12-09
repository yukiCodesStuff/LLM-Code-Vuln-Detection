    t.references :account
  end

  create_table :admin_user_jsons, force: true do |t|
    t.string :name
    t.string :settings, null: true, limit: 1024
    t.string :parent, null: true, limit: 1024
    t.string :spouse, null: true, limit: 1024
    t.string :configs, null: true, limit: 1024
    # MySQL does not allow default values for blobs. Fake it out with a
    # big varchar below.
    t.string :preferences, null: true, default: "", limit: 1024
    t.string :json_data, null: true, limit: 1024
    t.string :json_data_empty, null: true, default: "", limit: 1024
    t.text :params
    t.references :account
  end

  create_table :aircraft, force: true do |t|
    t.string :name
    t.integer :wheels_count, default: 0, null: false
    t.datetime :wheels_owned_at