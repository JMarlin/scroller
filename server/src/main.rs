use std::env;
use std ::net::{ TcpStream, TcpListener };
use std::io::{ Seek, SeekFrom, Read, Write };
use std::fs::{ File };
use std::str::{ FromStr };

const DEFAULT_PORT: &'static str = "8080";

#[derive(Debug)]
struct HttpRequest<'a> {
    pub method: &'a str,
    pub path: &'a str,
    pub protocol: &'a str,
    pub headers: Vec<(&'a str, &'a str)>,
    pub body: &'a str
}

impl<'a> HttpRequest<'a> {

    pub fn from_stream(tcp_stream: &TcpStream) -> HttpRequest<'a> {

        //TODO: Actually implement

        HttpRequest {
            method: "GET",
            path: "/", 
            protocol: "HTTP/1.1", 
            headers: vec![],
            body: ""
        }
    }

    pub fn print(&self) {

        println!("{self:?}");
    }
}

fn GetHttpRequest<'a>(tcp_stream: &TcpStream) -> HttpRequest<'a> {

    println!("Getting http request...");

    let req = HttpRequest::from_stream(&tcp_stream);

    println!("Done getting request");

    req
}

fn HttpSendStatic(tcp_stream: TcpStream, file_path: &str, mime_type: &str) {

    println!("Serving file '{file_path}' of type {mime_type}");

/* TODO
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

    res.send(tcp_stream);

    let mut buffer = [0; 2048];

    loop {

        let readCount = file.read(&buffer)?;

        tcp_stream.write(&buffer[..readCount]);

        if readCount < buffer.len() {

            break;
        }
    }
*/
}

fn IndexHandler(request: HttpRequest, tcp_stream: TcpStream) {
    HttpSendStatic(tcp_stream, "../client/index.html", "text/html");
}

fn StaticJSHandler(request: HttpRequest, tcp_stream: TcpStream) {
    HttpSendStatic(tcp_stream, "../client/client.js", "application/javascript");
}

fn StaticWASMHandler(request: HttpRequest, tcp_stream: TcpStream) {
    HttpSendStatic(tcp_stream, "../client/client.wasm", "application/wasm");
}

fn StaticGameWASMHandler(request: HttpRequest, tcp_stream: TcpStream) {

    println!("\nSTATIC GAME WASM HANDLER\n");
    HttpSendStatic(tcp_stream, "../client/game.wasm", "application/wasm");
}

fn NotFoundHandler(request: HttpRequest, tcp_stream: TcpStream) {

    let request_path = "/buttz"; //request.path;
    let body_message = format!("<center><h1>404: path {request_path} not found</h1></center>");
    let body_message_length = body_message.len();
    let body_message_length_str = format!("{body_message_length}");

    let res = HttpResponse {
        protocol: "HTTP/1.1",
        response_code: 404,
        response_code_description: "NOTFOUND",
        headers: vec![
            ("Content-Type", "text/html"),
            ("Content-Length", &body_message_length_str)
        ]
    };

    res.send(tcp_stream);
}

struct HttpResponse<'a> {
    pub protocol: &'a str,
    pub response_code: u32,
    pub response_code_description: &'a str,
    pub headers: Vec<(&'a str, &'a str)>
}

impl HttpResponse<'_> {

    pub fn send(&self, tcp_stream: TcpStream) { }
}

struct Endpoint {
    pub path: &'static str,
    pub method: &'static str,
    pub handler: fn(HttpRequest, TcpStream)
}

struct EndpointCollection {
    pub not_found_handler: fn(HttpRequest, TcpStream),
    pub endpoints: Vec<Endpoint>
}

fn main() {

    let endpoints = EndpointCollection {
        not_found_handler: NotFoundHandler,
        endpoints: vec![
            Endpoint { path: "/",             method: "GET", handler: IndexHandler          },
            Endpoint { path: "//client.js",   method: "GET", handler: StaticJSHandler       },
            Endpoint { path: "//client.wasm", method: "GET", handler: StaticWASMHandler     },
            Endpoint { path: "//game.wasm",   method: "GET", handler: StaticGameWASMHandler } ],
    };

    let port_string = env::var("FUNCTIONS_CUSTOMHANDLER_PORT")
        .or_else(|_| String::from_str(DEFAULT_PORT))
        .unwrap();

    println!("The port is: {port_string}");

    let listener =
        TcpListener::bind(format!("0.0.0.0:{port_string}"))
            .expect("Socket creation failed...");

    println!("Socket successfully created...");

    let incoming = listener.incoming();

    println!("Server listening...");

    for tcp_stream in incoming {

        let tcp_stream = tcp_stream
            .expect("Server accept failed...\n");

        println!("Server accepted a client...");

        let req = GetHttpRequest(&tcp_stream);
        req.print();

        let target_handler = endpoints.endpoints
            .iter()
            .find(|e| e.path == req.path)
            .map(|e| e.handler)
            .or_else(| | Some(endpoints.not_found_handler))
            .unwrap();

        target_handler(req, tcp_stream);
    }
}