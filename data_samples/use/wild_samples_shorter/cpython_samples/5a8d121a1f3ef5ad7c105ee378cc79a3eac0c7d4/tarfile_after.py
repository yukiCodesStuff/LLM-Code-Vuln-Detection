
            length, keyword = match.groups()
            length = int(length)
            if length == 0:
                raise InvalidHeaderError("invalid header")
            value = buf[match.end(2) + 1:match.start(1) + length - 1]

            # Normally, we could just use "utf-8" as the encoding and "strict"
            # as the error handler, but we better not take the risk. For