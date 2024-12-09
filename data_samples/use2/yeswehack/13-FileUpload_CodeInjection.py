from flask import Flask, render_template, request
import html
import customLog
from ignore.design import design
app = design.Design(Flask(__name__), __file__, 'snippet')

app = Flask(__name__)
app.config["UPLOAD_FOLDER"] = "share/"

@app.route('/share')
def sharedFiles():
    getFile = request.args.get("filename").replace('/','').replace('\\', '')
    file = open(app.config['UPLOAD_FOLDER'] + getFile, "r")

    return file.read() 

@app.route('/', methods = ['GET', 'POST'])
def index():
    customLog.log()

    if request.method == 'POST':
        f = request.files['file']
        f.save(app.config["UPLOAD_FOLDER"] + f.filename)
        
        return ('''<h2>File: succeeded!</h2><br>
        <a href="/">..Back</a><br>
        <a href="./share?filename=%s">See my file</a>''' % html.escape(f.filename))

    return render_template('index.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1337, debug=True)