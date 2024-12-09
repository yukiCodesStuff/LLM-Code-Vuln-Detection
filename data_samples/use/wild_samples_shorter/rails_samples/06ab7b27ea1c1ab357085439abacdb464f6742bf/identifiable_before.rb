
module ActiveStorage::Blob::Identifiable
  def identify
    update! content_type: identify_content_type, identified: true unless identified?
  end

  def identified?
    identified
        ""
      end
    end
end