
Defaults to `false`. Allows applications to opt into using `unsafe_load` on the `ActiveRecord::Coders::YAMLColumn`.

#### `config.active_record.raise_int_wider_than_64bit`

Defaults to `true`. Determines whether to raise an exception or not when
the PostgreSQL adapter is provided an integer that is wider than signed
64bit representation.

#### `ActiveRecord::ConnectionAdapters::Mysql2Adapter.emulate_booleans`

Controls whether the Active Record MySQL adapter will consider all `tinyint(1)` columns as booleans. Defaults to `true`.
