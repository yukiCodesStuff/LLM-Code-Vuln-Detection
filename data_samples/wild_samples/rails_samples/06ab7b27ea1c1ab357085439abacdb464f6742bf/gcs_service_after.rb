# frozen_string_literal: true

gem "google-cloud-storage", "~> 1.11"
require "google/cloud/storage"

module ActiveStorage
  # Wraps the Google Cloud Storage as an Active Storage service. See ActiveStorage::Service for the generic API
  # documentation that applies to all services.
  class Service::GCSService < Service
    def initialize(**config)
      @config = config
    end

    def upload(key, io, checksum: nil, content_type: nil, disposition: nil, filename: nil)
      instrument :upload, key: key, checksum: checksum do
        begin
          # GCS's signed URLs don't include params such as response-content-type response-content_disposition
          # in the signature, which means an attacker can modify them and bypass our effort to force these to
          # binary and attachment when the file's content type requires it. The only way to force them is to
          # store them as object's metadata.
          content_disposition = content_disposition_with(type: disposition, filename: filename) if disposition && filename
          bucket.create_file(io, key, md5: checksum, content_type: content_type, content_disposition: content_disposition)
        rescue Google::Cloud::InvalidArgumentError
          raise ActiveStorage::IntegrityError
        end
      end
    end

    def download(key, &block)
      if block_given?
        instrument :streaming_download, key: key do
          stream(key, &block)
        end
      else
        instrument :download, key: key do
          begin
            file_for(key).download.string
          rescue Google::Cloud::NotFoundError
            raise ActiveStorage::FileNotFoundError
          end
        end
      end
    end

    def update_metadata(key, content_type:, disposition: nil, filename: nil)
      instrument :update_metadata, key: key, content_type: content_type, disposition: disposition do
        file_for(key).update do |file|
          file.content_type = content_type
          file.content_disposition = content_disposition_with(type: disposition, filename: filename) if disposition && filename
        end
      end
    end

    def download_chunk(key, range)
      instrument :download_chunk, key: key, range: range do
        begin
          file_for(key).download(range: range).string
        rescue Google::Cloud::NotFoundError
          raise ActiveStorage::FileNotFoundError
        end
      end
    end

    def delete(key)
      instrument :delete, key: key do
        begin
          file_for(key).delete
        rescue Google::Cloud::NotFoundError
          # Ignore files already deleted
        end
      end
    end

    def delete_prefixed(prefix)
      instrument :delete_prefixed, prefix: prefix do
        bucket.files(prefix: prefix).all do |file|
          begin
            file.delete
          rescue Google::Cloud::NotFoundError
            # Ignore concurrently-deleted files
          end
        end
      end
    end

    def exist?(key)
      instrument :exist, key: key do |payload|
        answer = file_for(key).exists?
        payload[:exist] = answer
        answer
      end
    end

    def url(key, expires_in:, filename:, content_type:, disposition:)
      instrument :url, key: key do |payload|
        generated_url = file_for(key).signed_url expires: expires_in, query: {
          "response-content-disposition" => content_disposition_with(type: disposition, filename: filename),
          "response-content-type" => content_type
        }
# frozen_string_literal: true

gem "google-cloud-storage", "~> 1.11"
require "google/cloud/storage"

module ActiveStorage
  # Wraps the Google Cloud Storage as an Active Storage service. See ActiveStorage::Service for the generic API
  # documentation that applies to all services.
  class Service::GCSService < Service
    def initialize(**config)
      @config = config
    end

    def upload(key, io, checksum: nil, content_type: nil, disposition: nil, filename: nil)
      instrument :upload, key: key, checksum: checksum do
        begin
          # GCS's signed URLs don't include params such as response-content-type response-content_disposition
          # in the signature, which means an attacker can modify them and bypass our effort to force these to
          # binary and attachment when the file's content type requires it. The only way to force them is to
          # store them as object's metadata.
          content_disposition = content_disposition_with(type: disposition, filename: filename) if disposition && filename
          bucket.create_file(io, key, md5: checksum, content_type: content_type, content_disposition: content_disposition)
        rescue Google::Cloud::InvalidArgumentError
          raise ActiveStorage::IntegrityError
        end
      end
    end

    def download(key, &block)
      if block_given?
        instrument :streaming_download, key: key do
          stream(key, &block)
        end
      else
        instrument :download, key: key do
          begin
            file_for(key).download.string
          rescue Google::Cloud::NotFoundError
            raise ActiveStorage::FileNotFoundError
          end
        end
      end
    end

    def update_metadata(key, content_type:, disposition: nil, filename: nil)
      instrument :update_metadata, key: key, content_type: content_type, disposition: disposition do
        file_for(key).update do |file|
          file.content_type = content_type
          file.content_disposition = content_disposition_with(type: disposition, filename: filename) if disposition && filename
        end
      end
    end

    def download_chunk(key, range)
      instrument :download_chunk, key: key, range: range do
        begin
          file_for(key).download(range: range).string
        rescue Google::Cloud::NotFoundError
          raise ActiveStorage::FileNotFoundError
        end
      end
    end

    def delete(key)
      instrument :delete, key: key do
        begin
          file_for(key).delete
        rescue Google::Cloud::NotFoundError
          # Ignore files already deleted
        end
      end
    end

    def delete_prefixed(prefix)
      instrument :delete_prefixed, prefix: prefix do
        bucket.files(prefix: prefix).all do |file|
          begin
            file.delete
          rescue Google::Cloud::NotFoundError
            # Ignore concurrently-deleted files
          end
        end
      end
    end

    def exist?(key)
      instrument :exist, key: key do |payload|
        answer = file_for(key).exists?
        payload[:exist] = answer
        answer
      end
    end

    def url(key, expires_in:, filename:, content_type:, disposition:)
      instrument :url, key: key do |payload|
        generated_url = file_for(key).signed_url expires: expires_in, query: {
          "response-content-disposition" => content_disposition_with(type: disposition, filename: filename),
          "response-content-type" => content_type
        }