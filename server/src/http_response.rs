use std::net::TcpStream;

pub struct HttpResponse<'a> {
    pub protocol: &'a str,
    pub response_code: u32,
    pub response_code_description: &'a str,
    pub headers: Vec<(&'a str, &'a str)>
}

impl HttpResponse<'_> {

    pub fn send(&self, tcp_stream: TcpStream) { }
}
