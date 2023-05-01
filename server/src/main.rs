use std::env;
use std ::net::{ TcpStream, TcpListener };
use std::io::{ Seek, SeekFrom, Read, Write };
use std::fs::{ File };
use std::str::{ FromStr };

mod http_request; use http_request::*;
mod http_response; use http_response::*;
mod endpoint; use endpoint::*;

const DEFAULT_PORT: &'static str = "8080";

fn get_http_request<'a>(tcp_stream: &TcpStream) -> HttpRequest {

    println!("Getting http request...");

    let req = HttpRequest::from_stream(&tcp_stream)
        .expect("Unable to read http request");

    println!("Done getting request");

    req
}

fn http_send_static(mut tcp_stream: TcpStream, file_path: &str, mime_type: &str) {

    println!("Serving file '{file_path}' of type {mime_type}");

    let mut file = File::open(file_path)
        .expect(&format!("Unable to open file {file_path}"));

    let length = file
        .seek(SeekFrom::End(0))
        .expect("Unable to sek into static file");

    file.seek(SeekFrom::Start(0))
        .expect("Unable to rewind static file");

    let length_string = format!("{length}");

    let res = HttpResponse {
        protocol:  "HTTP/1.1",
        response_code: 200,
        response_code_description: "OK",
        headers: vec![
            ("Content-Type", &mime_type),
            ("Content-Length", &length_string),
            ("Cross-Origin-Opener-Policy", "same-origin"),
            ("Cross-Origin-Embedder-Policy", "require-corp")
        ],
        body: ""
    };

    res.send(&mut tcp_stream);

    let mut buffer = [0; 2048];

    loop {

        let read_count = file
            .read(&mut buffer)
            .expect("Unable to read from file buffer");

        tcp_stream
            .write(&buffer[..read_count])
            .expect("Unable to write to TCP stream");

        if read_count < buffer.len() {

            break;
        }
    }
}

fn index_handler(_request: HttpRequest, tcp_stream: TcpStream) {
    http_send_static(tcp_stream, "../client/index.html", "text/html");
}

fn static_js_handler(_request: HttpRequest, tcp_stream: TcpStream) {
    http_send_static(tcp_stream, "../client/client.js", "application/javascript");
}

fn static_wasm_handler(_request: HttpRequest, tcp_stream: TcpStream) {
    http_send_static(tcp_stream, "../client/client.wasm", "application/wasm");
}

fn static_game_wasm_handler(_request: HttpRequest, tcp_stream: TcpStream) {

    println!("\nSTATIC GAME WASM HANDLER\n");
    http_send_static(tcp_stream, "../client/game.wasm", "application/wasm");
}

fn not_found_handler(request: HttpRequest, mut tcp_stream: TcpStream) {

    let request_path = request.path;
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
        ],
        body: &body_message
    };

    res.send(&mut tcp_stream);
}

fn main() {

    let endpoints = EndpointCollection {
        not_found_handler: not_found_handler,
        endpoints: vec![
            Endpoint { path: "/",             method: HttpMethod::Get, handler: index_handler          },
            Endpoint { path: "//client.js",   method: HttpMethod::Get, handler: static_js_handler       },
            Endpoint { path: "//client.wasm", method: HttpMethod::Get, handler: static_wasm_handler     },
            Endpoint { path: "//game.wasm",   method: HttpMethod::Get, handler: static_game_wasm_handler } ],
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

        let req = get_http_request(&tcp_stream);
        req.print();

        /*
        let target_handler = endpoints.endpoints
            .iter()
            .find(|e| e.path == req.path && e.method == req.method)
            .map(|e| e.handler)
            .or_else(| | Some(endpoints.not_found_handler))
            .unwrap();
        */

        let req_path = if req.path == "/" { "/index.html" } else { &req.path };

        http_send_static(tcp_stream, &format!("../client{req_path}"), if req_path.ends_with(".html") { "text/html" } else if req_path.ends_with(".wasm") { "application/wasm" } else if req_path.ends_with(".js") { "application/javascript" } else { "text/plain" });

        //target_handler(req, tcp_stream);
    }
}