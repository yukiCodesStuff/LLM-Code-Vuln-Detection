
  def show
    if key = decode_verified_key
      serve_file disk_service.path_for(key[:key]), content_type: key[:content_type], disposition: key[:disposition]
    else
      head :not_found
    end
  rescue Errno::ENOENT