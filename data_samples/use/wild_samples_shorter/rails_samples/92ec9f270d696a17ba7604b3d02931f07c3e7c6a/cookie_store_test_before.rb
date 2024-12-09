    end

    def get_session_id
      render plain: "id: #{request.session.id}"
    end

    def get_class_after_reset_session
      reset_session