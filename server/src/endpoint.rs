use std::net::TcpStream;
use crate::{HttpRequest, http_request::HttpMethod};

pub struct Endpoint {
    pub path: &'static str,
    pub method: HttpMethod,
    pub handler: fn(HttpRequest, TcpStream)
}

pub struct EndpointCollection {
    pub not_found_handler: fn(HttpRequest, TcpStream),
    pub endpoints: Vec<Endpoint>
}