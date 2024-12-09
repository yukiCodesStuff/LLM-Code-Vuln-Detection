    end

    def get_session_id
      render plain: "id: #{request.session.id&.public_id}"
    end

    def get_class_after_reset_session
      reset_session