# frozen_string_literal: true

require "yaml"
require "tempfile"
require "active_support/message_encryptor"

module Rails
  # Greatly inspired by Ara T. Howard's magnificent sekrets gem. 😘
        end

        def writing(contents)
          file_name = "#{File.basename(path)}.#{Process.pid}"

          Tempfile.create(["", "-" + file_name]) do |tmp_file|
            tmp_path = Pathname.new(tmp_file)
            tmp_path.binwrite contents

            yield tmp_path

            updated_contents = tmp_path.binread

            write(updated_contents) if updated_contents != contents
          end
        end

        def encryptor
          @encryptor ||= ActiveSupport::MessageEncryptor.new([ key ].pack("H*"), cipher: @cipher)