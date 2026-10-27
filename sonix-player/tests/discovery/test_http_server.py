#!/usr/bin/env python3
"""Real local TLS server; no external network or bypassed verification."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import os, ssl, subprocess, tempfile, threading, sys
class Handler(BaseHTTPRequestHandler):
    def log_message(self,*args): pass
    def do_GET(self):
        path=self.path[1:];status=200;body=b'0123456789';headers={}
        if path in ('redirect','loop','downgrade'):
            self.send_response(302);self.send_header('Location', {'redirect':'/ok','loop':'/loop','downgrade':'http://localhost:1/ok'}[path]);self.end_headers();return
        if path in ('range','bad-range','bad-total'):
            assert self.headers.get('Range')=='bytes=4-'
            status=206;body=body[4:];headers['Content-Range']={'range':'bytes 4-9/10','bad-range':'bytes 3-8/10','bad-total':'bytes 4-10/11'}[path]
        if path=='chunked':headers['Transfer-Encoding']='chunked'
        if path=='encoded':headers['Content-Encoding']='gzip'
        if path=='missing':status=404
        self.send_response(status)
        self.send_header('Content-Length', '11' if path=='length' else str(len(body)))
        for key,value in headers.items():self.send_header(key,value)
        self.end_headers()
        try:self.wfile.write(body)
        except (BrokenPipeError,ssl.SSLError):pass
with tempfile.TemporaryDirectory(prefix='sonix-tls-test-') as temp:
    cert=Path(temp)/'cert.pem';key=Path(temp)/'key.pem'
    subprocess.run(['openssl','req','-x509','-newkey','rsa:2048','-nodes','-keyout',str(key),'-out',str(cert),'-days','1','-subj','/CN=localhost','-addext','subjectAltName=DNS:localhost'],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    server=ThreadingHTTPServer(('127.0.0.1',0),Handler);ctx=ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER);ctx.load_cert_chain(cert,key);server.socket=ctx.wrap_socket(server.socket,server_side=True)
    thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
    env=os.environ.copy();env['TEST_CA']=str(cert)
    try:subprocess.run([sys.argv[1],f'https://localhost:{server.server_port}',f'https://127.0.0.1:{server.server_port}'],check=True,env=env)
    finally:server.shutdown();thread.join();server.server_close()
