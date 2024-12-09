
module ActiveStorage::Blob::Identifiable
  def identify
    unless identified?
      update! content_type: identify_content_type, identified: true
      update_service_metadata
    end
  end

  def identified?
    identified
        ""
      end
    end

    def update_service_metadata
      service.update_metadata key, service_metadata if service_metadata.any?
    end
end