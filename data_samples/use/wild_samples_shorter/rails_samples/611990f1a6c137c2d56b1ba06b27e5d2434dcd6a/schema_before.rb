    t.references :account
  end

  create_table :aircraft, force: true do |t|
    t.string :name
    t.integer :wheels_count, default: 0, null: false
    t.datetime :wheels_owned_at