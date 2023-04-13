use std::net::TcpStream;
use crate::HttpRequest;

pub struct Endpoint {
    pub path: &'static str,
    pub method: &'static str,
    pub handler: fn(HttpRequest, TcpStream)
}

pub struct EndpointCollection {
    pub not_found_handler: fn(HttpRequest, TcpStream),
    pub endpoints: Vec<Endpoint>
}