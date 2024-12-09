from flask import Flask, render_template, Response, request
from datetime import date
import base64 as b64
import pickle

app = Flask(__name__)

def User_RedirectTo(d):
    ##Handle the user data and redirect
    #Code...
    return "<h2>Redirecting you!</h2>"

class CreateData(object,):
    def __init__(self, id, name, date):
        self.id = id
        self.name = name
        self.date = date

    def __str__(self):
        return str(self.__dict__)


@app.route('/', methods = ['GET', 'POST'])
def index():
    resp = Response()
    
    dataCookie = request.cookies.get('userData')

    if dataCookie is not None:
        try:
            data = b64.b64decode(bytes(dataCookie, 'UTF-8'))
            data = pickle.loads(data)
            return User_RedirectTo(data)
        
        except:
            return render_template('index.html', result="<h2>Invalid data...</h2>")

    else:
        newData = CreateData(None, 'guest', date.today().strftime('%d/%m/%Y'))
        newData = bytes(str(newData), 'UTF-8')
        resp.set_cookie('userData', b64.b64encode(newData))
        
        return resp

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1337, debug=True)