
  def show
    if key = decode_verified_key
      serve_file disk_service.path_for(key), content_type: params[:content_type], disposition: params[:disposition]
    else
      head :not_found
    end
  rescue Errno::ENOENT