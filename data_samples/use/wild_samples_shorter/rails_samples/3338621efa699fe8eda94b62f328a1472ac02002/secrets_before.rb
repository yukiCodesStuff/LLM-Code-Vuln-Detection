# frozen_string_literal: true

require "yaml"
require "active_support/message_encryptor"

module Rails
  # Greatly inspired by Ara T. Howard's magnificent sekrets gem. 😘
        end

        def writing(contents)
          tmp_file = "#{File.basename(path)}.#{Process.pid}"
          tmp_path = File.join(Dir.tmpdir, tmp_file)
          IO.binwrite(tmp_path, contents)

          yield tmp_path

          updated_contents = IO.binread(tmp_path)

          write(updated_contents) if updated_contents != contents
        ensure
          FileUtils.rm(tmp_path) if File.exist?(tmp_path)
        end

        def encryptor
          @encryptor ||= ActiveSupport::MessageEncryptor.new([ key ].pack("H*"), cipher: @cipher)