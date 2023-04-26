use std::net::TcpStream;
use std::io::ErrorKind;
use std::io::{ Result, Error, BufReader, Read };
use std::str;

#[derive(PartialEq)]
#[derive(Debug)]
pub enum HttpMethod {
    Get,
    Post,
    Put,
    Delete,
    Update
}

impl HttpMethod {
    pub fn from_str(method_text: &str) -> Result<HttpMethod> { match method_text {
        "GET"    => Ok(HttpMethod::Get),
        "POST"   => Ok(HttpMethod::Post),
        "PUT"    => Ok(HttpMethod::Put),
        "DELETE" => Ok(HttpMethod::Delete),
        "UPDATE" => Ok(HttpMethod::Update),
        _ => Err(Error::new(ErrorKind::InvalidInput, format!("Unknown http method '{method_text}'")))
    } }
}

#[derive(Debug)]
pub struct HttpRequest {
    pub method: HttpMethod,
    pub path: String,
    pub protocol: String,
    pub headers: Vec<(String, String)>,
    pub body: String
}

impl HttpRequest {

    fn not_eol_or_space(b: &Result<u8>) -> bool { match b {
        Err(_) | Ok(b' ') | Ok(b'\r') | Ok(b'\n') => false,
        _ => true
    } }

    fn as_str_or_err<'b>(type_name: &'b str, bytes: &'b [u8]) -> Result<&'b str> {
        match str::from_utf8(bytes) {
            Ok(v) => Ok(v),
            _ => Err(Error::new(
                ErrorKind::InvalidData,
                format!("Could not parse {type_name}") ))
    } }

    pub fn from_stream(tcp_stream: &TcpStream) -> Result<HttpRequest> {

        let mut reader_bytes = BufReader::new(tcp_stream).bytes();

        println!("reading buf to request...");

        let method_bytes = reader_bytes
            .by_ref()
            .take_while(Self::not_eol_or_space)
            .filter_map(|r| r.ok())
            .collect::<Vec<u8>>();

        let method_text = Self::as_str_or_err("request method", &method_bytes)?;
        let method_enum = HttpMethod::from_str(method_text)?;

        let path_bytes = reader_bytes
            .by_ref()
            .take_while(Self::not_eol_or_space)
            .filter_map(|r| r.ok())
            .collect::<Vec<u8>>();

        let path_text = Self::as_str_or_err("request path", &path_bytes)?;

        let protocol_bytes = reader_bytes
            .by_ref()
            .take_while(Self::not_eol_or_space)
            .filter_map(|r| r.ok())
            .collect::<Vec<u8>>();

        let protocol_text = Self::as_str_or_err("request protocol", &protocol_bytes)?;

        let http_request = HttpRequest {
            method: method_enum,
            path: path_text.to_string(),
            protocol: protocol_text.to_string(),
            headers: vec![],
            body: "".to_string()
        };

        Ok(http_request)
    }

    pub fn print(&self) {

        println!("{self:?}");
    }
}