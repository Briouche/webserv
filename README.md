*This project has been created as part of the 42 curriculum by \<lecartuy>[, \<stdevis>[, \<brcoppie>]].*

# Webserv

## Description

**webserv** is a fully functional HTTP/1.1 server written in C++98, developed as part of the 42 school curriculum. The goal of the project is to gain a deep understanding of how web servers work by building one from scratch - without relying on any existing server framework or high-level networking library.

The server handles multiple simultaneous client connections using non-blocking I/O multiplexing (`select`, `poll`, or `epoll`). It parses and processes HTTP requests, serves static files, handles file uploads, and supports CGI execution for dynamic content generation.

Key features include:

- Full HTTP/1.1 request parsing (GET, POST, DELETE)
- Virtual hosting with multiple server blocks
- Custom error pages and configurable status responses
- File upload handling
- CGI support (e.g., Python, PHP scripts)
- Configurable routing with location blocks
- Non-blocking I/O - the server never blocks on a single client
- A configuration file inspired by NGINX syntax
- Support cookies and session management

---

## Instructions

### Requirements


- A C++98-compatible compiler (e.g., `g++` or `clang++`)
- A Unix-like operating system (Linux or macOS)
- `make`

### Compilation

```bash
make
```

This produces the `webserv` binary at the root of the repository.

### Running the server

```bash
./webserv [configuration_file]
```

### Super tester (rigorous)

A rigorous black-box tester is available at `tests/super_tester.py`. It will start the server, run a suite of protocol/feature/edge-case tests, and print a detailed pass/fail report.

```bash
make
./tests/super_tester.py --config server.config
```

If you already have the server running in another terminal:

```bash
./tests/super_tester.py --no-start
```

### Configuration file

The configuration file controls all server behaviour. Below is a minimal example:

```nginx
server {
    listen					8080;
    server_name				localhost;
    root        			./root;
    client_max_body_size 	1000;

    location / {
        allowed_methods     GET POST;
        autoindex   		on;
		index       		index.html;
    }

    location /uploads {
        allowed_methods     POST;
        upload_path  		./root/uploads;
    }

    location /cgi-bin {
        autoindex 			on;
        allowed_methods 	GET POST;
        root 				./cgi-bin;
        cgi_pass 			.php /usr/bin/php-cgi;
		cgi_pass 			.py /usr/bin/python3;
    }

    error_page				404 /errors/error404.html;
}
```

### Stopping the server

```bash
Ctrl+C
```

---

## Resources

### HTTP Protocol & Standards

- [RFC 2616 - Hypertext Transfer Protocol HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616#section-1) - The foundational specification for HTTP/1.1. Essential reading for understanding request/response structure, headers, status codes, and connection management.

- [The Common Gateway Interface (CGI) Version 1.1](https://datatracker.ietf.org/doc/html/rfc3875) - Foundational specifications for CGI, reference for how our CGI behaves

### Guides & Articles

- [HackMD - webserv guide by laian](https://hackmd.io/@laian/SJZHcOsmT) - A practical walkthrough of the webserv project covering implementation details, I/O multiplexing, and configuration parsing.

- [Tutorialspoint - C++ web programming: CGI](https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm) - Guide to building your first simple CGI

### Video Tutorials

- [Building a Web Server in C - YouTube](https://www.youtube.com/watch?v=D26sUZ6DHNQ) - A video tutorial walking through the fundamentals of socket programming and HTTP server construction in C/C++.

- [99% of Developers Don't Get Sockets - Youtube](https://www.youtube.com/watch?v=D26sUZ6DHNQ) - A video tutorial about how sockets work and how they are used

### AI Usage

AI (Claude by Anthropic) was used at various stages of this project:

- **Debugging:** Identifying subtle bugs in request parsing, socket handling, and edge cases in the HTTP state machine.
- **Understanding HTTP concepts:** Clarifying ambiguous sections of the RFC (chunked transfer encoding, persistent connections, header semantics).
- **Code suggestions:** Exploring idiomatic C++98 patterns for non-blocking I/O and buffer management.
- **Documentation:** Drafting and structuring this README.
- **HTML, Python & PHP:** Help with the creation of cgi programs or html pages to show the webserver works as expected

AI was used as a reference and sounding board - all design decisions, architecture choices, and final code were written and validated by the team.