use std::{net::TcpStream, io::Write};

pub struct HttpResponse<'a> {
    pub protocol: &'a str,
    pub response_code: u32,
    pub response_code_description: &'a str,
    pub headers: Vec<(&'a str, &'a str)>,
    pub body: &'a str
}

impl HttpResponse<'_> {

    pub fn send(&self, tcp_stream: &mut TcpStream) {

        let line_one = format!(
            "{0} {1} {2}\r\n",
            self.protocol,
            self.response_code,
            self.response_code_description );

        tcp_stream.write(line_one.as_bytes())
            .expect("Failed to write response description to TCP stream");

        for header in &self.headers {

            let header_text =
                format!("{0}: {1}\r\n", header.0, header.1);

            tcp_stream.write(header_text.as_bytes())
                .expect("Unable to write header to TCP stream");
        }

        tcp_stream.write("\r\n".as_bytes())
            .expect("Unable to write body separator to TCP stream");

        tcp_stream.write(self.body.as_bytes())
            .expect("Unable to write message body to TCP stream");
    }
}
