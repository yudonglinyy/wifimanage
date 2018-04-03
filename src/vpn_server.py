#coding:utf-8

from flask import Flask
from flask import request
from flask import send_from_directory
from flask import make_response
import json

app = Flask(__name__)

from random import randrange
from pyDes import *
import base64
import os
import time

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import or_, and_
from vpn_auth_modules import Box, Server, init_db

import pymysql
pymysql.install_as_MySQLdb()

DB_CONNECT_STRING = "mysql+mysqldb://root:123@127.0.0.1:3306/vpn"
engine = create_engine(DB_CONNECT_STRING, encoding="utf-8", echo=True)
DB_Session = sessionmaker(bind=engine)
session = DB_Session()

init_db(engine)

def set_realkey(key):
    return key+"peng"

@app.route('/auth/api/v1/token', methods=['GET', 'POST'])
def auth_token():
    # data = request.form.get("data").replace(" ", "+")
    data = request.form.get("data")
    tokendata_key = request.form.get("key")

    invaild_jsondata = {"status":"1", "msg":"invaild post data", "key":"NULL", "url": "NULL"}

    if not data or not tokendata_key:
        print("error form {}".format(data))
        return make_response(json.dumps(invaild_jsondata))

    if 4 != len(tokendata_key):
        print("error key: %s" % tokendata_key)
        return make_response(json.dumps(invaild_jsondata))
    else:
        tokendata_key = set_realkey(tokendata_key)

    k = des(tokendata_key, ECB, "00000000", pad=None, padmode=PAD_PKCS5)
    token_json = json.loads(k.decrypt(base64.b64decode(data)).decode('utf-8'))
    print(token_json)
    print(data)
    print(tokendata_key)

    ip = token_json.get('ip')
    mac = token_json.get('mac')
    hashstr = token_json.get('hash')
    version = token_json.get('version')
    reqtime = token_json.get('reqtime')

    if not ip or not mac or not hashstr or not version or not reqtime:
        print('test')
        return make_response(json.dumps(invaild_jsondata))

   # The time error is 1 hours
    if abs(int(time.time())-int(reqtime) > 3600 * 1):
        return make_response(json.dumps(invaild_jsondata))

    nowtimestr = time.strftime('%Y-%m-%d-%H:%M:%S', time.localtime(time.time()))
    urlbase = "http://127.0.0.1:5000"
    downloaddir = "Download"
    cert_dirname = "{}-{}".format(nowtimestr, randrange(10000000,100000000))
    tarfilename = "{}.tar.gz".format(cert_dirname)
    url = "{}/{}/{}".format(urlbase, downloaddir, tarfilename)
    url_key = str(randrange(10000)).zfill(4)
    realkey = set_realkey(url_key)

    item = session.query(Box).filter(and_(Box.mac == mac, Box.cert_eigenvalue == hashstr)).first()
    if item:
        static_cert_dirname = item.cert_path.split("\\")
        print("static_cert_dirname:{}".format(static_cert_dirname))
    else:
        return make_response(json.dumps(invaild_jsondata))

    # TODO select db
    static_cert_dirname = "1"
    os.chdir(downloaddir)
    os.system("cp -R {} {}".format(static_cert_dirname, cert_dirname))
    res = os.system('tar -zcf - {} | openssl des3 -salt -k {} -out {}'.format(cert_dirname, realkey, tarfilename))
    if res != 0:
        print("os.system failed")
        return make_response(json.dumps(invaild_jsondata))
    os.system("rm -rf {}".format(cert_dirname))
    os.chdir("../")

    k2 = des(realkey, ECB, "00000000", pad=None, padmode=PAD_PKCS5)
    en_url = base64.b64encode(k2.encrypt(url)).decode('utf-8')

    resultjson = {
        "status":"0",
        "url": en_url,
        "key": url_key,
        "msg":"succ"
    }

    print(resultjson)
    rst = make_response(json.dumps(resultjson))
    rst.headers['Access-Control-Allow-Origin'] = '*'
    return rst



@app.route('/Download/<filename>', methods=['GET'])
def download_cert(filename):
    print("download {}".format(filename))
    return send_from_directory("Download/", filename, as_attachment=True)


@app.route('/auth/api/v1/connect', methods=['GET', 'POST'])
def auth_user():
    info = request.form.get("info")
    hashstr = request.form.get("hash")
    key = request.form.get("key")
    serverid = request.form.get("serverid")

    invaild_jsondata = {"status":"1", "msg":"invaild post data"}

    if not info or not hashstr or not key or not serverid:
        return make_response(json.dumps(invaild_jsondata))


    resultjson = dict()
    if request.method == 'GET':
        invaild_jsondata['msg']='This is a get request that does not match'
        return make_response(json.dumps(invaild_jsondata))

    realkey = set_realkey(key)
    k = des(realkey, ECB, b"00000000", pad=None, padmode=PAD_PKCS5)
    deinfo = k.decrypt(base64.b64decode(info)).decode('utf-8').split('|')

    if len(deinfo) != 4:
        return make_response(json.dumps(invaild_jsondata))

    mac = deinfo[0]
    ip = deinfo[1]
    version = deinfo[2]
    reqtime = deinfo[3]

    print(time.localtime(time.time()))
    print(time.localtime(int(reqtime)))
    print(time.time())
    print(reqtime)
    # The time error is 1 hours
    if abs(int(time.time())-int(reqtime) > 3600 * 1):
        return make_response(json.dumps(invaild_jsondata))


    item = session.query(Box).filter(and_(Box.mac == mac, Box.cert_eigenvalue == hashstr)).first()
    if item:
        ip = item.server_ip
        cid = item.cid
        static_cert_dirname = item.cert_path.split("\\")
        print("static_cert_dirname:{}".format(static_cert_dirname))
    else:
        return make_response(json.dumps(invaild_jsondata))


    resultjson['status']='0'
    resultjson['msg']='ok'
    resultjson['ip']=ip
    resultjson['cid']=cid
    print(resultjson)

    rst = make_response(json.dumps(resultjson))
    rst.headers['Access-Control-Allow-Origin'] = '*'
    return rst


if __name__ == '__main__':
    app.run(
        host = '0.0.0.0',
        port = 5000,
        debug = True
    )