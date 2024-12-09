@rem if exist tcl-8.6.1.0 rd /s/q tcl-8.6.1.0
@rem if exist tk-8.6.1.0 rd /s/q tk-8.6.1.0
@rem if exist db-4.4.20 rd /s/q db-4.4.20
@rem if exist openssl-1.0.1e rd /s/q openssl-1.0.1g
@rem if exist sqlite-3.7.12 rd /s/q sqlite-3.7.12    

@rem bzip
if not exist bzip2-1.0.6 (
)

@rem OpenSSL
if not exist openssl-1.0.1g (
    rd /s/q openssl-1.0.1e
    svn export http://svn.python.org/projects/external/openssl-1.0.1g
)

@rem tcl/tk
if not exist tcl-8.6.1.0 (