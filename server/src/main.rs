use std::io;
use std::time;
use std ::net::{ TcpListener, TcpStream };
use std::io::{ Seek, SeekFrom, Read, Write };
use std::fs:: { File };
use std::thread;

const DEFAULT_PORT: &'static str = "8080";

fn GetHttpRequest(clientSock: i32) -> HttpRequest {

    println!("Getting http request...");

    let req = ReadHttpRequest(SocketLoader(clientSock));

    println!("Done getting request");

    req
}

fn HttpSendStatic(tcpStream: TcpStream, filePath: &str, mimeType: &str) {

    let mut lengthStr = [0; 32];
    let file = File::open(filePath)?;

    let length = file
        .seek(SeekFrom::End(0))
        .expect("Unable to sek into static file");

    file.seek(SeekFrom::Start(0))
        .expect("Unable to rewind static file");

    intToStrN(length, &lengthStr, 32);

    let res = HttpResponse {
        protocol:  "HTTP/1.1",
        responseCode: 200,
        responseDescription: "OK",
        headers: [
            HttpHeader::new("Content-Type", mimeType),
            HttpHeader::new("Content-Length", lengthStr),
            HttpHeader::new("Cross-Origin-Opener-Policy", "same-origin"),
            HttpHeader::new("Cross-Origin-Embedder-Policy", "require-corp")
        ],
        body: ""
    };

    res.send(tcpStream);

    let mut buffer = [0; 2048];

    loop {

        let readCount = file.read(&buffer)?;

        tcpStream.write(&buffer[..readCount]);

        if readCount < buffer.len() {

            break;
        }
    }
}

fn IndexHandler(request: &HttpRequest, tcpStream: TcpStream) {
    HttpSendStatic(tcpStream, "../client/index.html", "text/html");
}

fn StaticJSHandler(request: &HttpRequest, tcpStream: TcpStream) {
    HttpSendStatic(tcpStream, "../client/client.js", "application/javascript");
}

fn StaticWASMHandler(request: &HttpRequest, tcpStream: TcpStream) {
    HttpSendStatic(tcpStream, "../client/client.wasm", "application/wasm");
}

fn StaticGameWASMHandler(request: &HttpRequest, tcpStream: TcpStream) {

    println!("\nSTATIC GAME WASM HANDLER\n");
    HttpSendStatic(tcpStream, "../client/game.wasm", "application/wasm");
}

fn NotFoundHandler(request: &HttpRequest, tcpStream: TcpStream) {

    let bodyMessage = "<center><h1>404: path {request.path} not found</h1></center>";

    let res = HttpResponse {
        protocol: "HTTP/1.1",
        responseCode: 404,
        responseDescription: "NOTFOUND",
        headers: [
            HttpHeader::new("Content-Type", "text/html"),
            HttpHeader::new("Content-Length", "{bodyMessage.len()}")
        ]
    };

    res.send(tcpStream);
}

fn main() {

    todo!("Copy from main.c");
}