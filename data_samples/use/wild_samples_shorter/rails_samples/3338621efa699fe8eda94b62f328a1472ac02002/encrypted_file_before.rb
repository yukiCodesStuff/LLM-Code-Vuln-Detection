# frozen_string_literal: true

require "pathname"
require "tmpdir"
require "active_support/message_encryptor"

module ActiveSupport
  class EncryptedFile

    private
      def writing(contents)
        tmp_file = "#{Process.pid}.#{content_path.basename.to_s.chomp('.enc')}"
        tmp_path = Pathname.new File.join(Dir.tmpdir, tmp_file)
        tmp_path.binwrite contents

        yield tmp_path

        updated_contents = tmp_path.binread

        write(updated_contents) if updated_contents != contents
      ensure
        FileUtils.rm(tmp_path) if tmp_path&.exist?
      end


      def encrypt(contents)