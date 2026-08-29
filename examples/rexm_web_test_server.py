import os
from http.server import BaseHTTPRequestHandler, HTTPServer

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        # The path we get is an absolute path on the server (which is not an absolute path on our local filesystem)
        # Remove the leading slash so that its a relative path
        path = self.path.lstrip("/")

        # Set the correct content type
        extension = os.path.splitext(self.path)[1];
        if (extension == ".html"): content_type = 'text/html'
        elif (extension == ".js"): content_type = 'text/javascript'
        elif (extension == ".wasm"): content_type = 'application/wasm'
        elif (extension == ".data"): content_type = 'application/octet-stream'
        else:
            # Some content type that we just ignore
            self.send_response(501)
            return

        with open(path, "rb") as f:
            content = f.read()

        self.send_response(200)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(content)))
        self.end_headers()
        self.wfile.write(content)

    # The only POST request we get is the one we send when we're saving the `exName.log` file
    def do_POST(self):
        path = self.path.lstrip("/")

        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)

        with open(path, "wb") as f:
            content = f.write(body)

        self.send_response(200)
        self.end_headers()

if __name__ == "__main__":
    with HTTPServer(("", 38080), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            # Ctrl + C was pressed
            pass
