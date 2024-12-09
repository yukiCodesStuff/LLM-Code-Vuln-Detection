# frozen_string_literal: true

require "pathname"
require "tempfile"
require "active_support/message_encryptor"

module ActiveSupport
  class EncryptedFile

    private
      def writing(contents)
        Tempfile.create(["", "-" + content_path.basename.to_s.chomp(".enc")]) do |tmp_file|
          tmp_path = Pathname.new(tmp_file)
          tmp_path.binwrite contents

          yield tmp_path

          updated_contents = tmp_path.binread

          write(updated_contents) if updated_contents != contents
        end
      end


      def encrypt(contents)