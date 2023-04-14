use std::net::TcpStream;
use std::io::Result;

#[derive(Debug)]
pub struct HttpRequest<'a> {
    pub method: &'a str,
    pub path: &'a str,
    pub protocol: &'a str,
    pub headers: Vec<(&'a str, &'a str)>,
    pub body: &'a str
}

impl<'a> HttpRequest<'a> {

    pub fn from_stream(tcp_stream: &TcpStream) -> Result<HttpRequest<'a>> {

        //TODO: Actually implement

        Ok(HttpRequest {
            method: "GET",
            path: "/dingbatz", 
            protocol: "HTTP/1.1", 
            headers: vec![],
            body: ""
        } )
    }

    pub fn print(&self) {

        println!("{self:?}");
    }
}