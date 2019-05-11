//========================================================================
// Zinc - Web Server
// Copyright (c) 2019, Pascal Levy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    // THE SOFTWARE.
//========================================================================

#ifndef _WIN32
#include <unistd.h>
#include <poll.h>
#include <sys/wait.h>
#endif
#include <iostream>

#include "../misc/portability.h"
#include "../misc/logger.h"
#include "zinc.h"
#include "resource_script.h"

//========================================================================
// ResourceScript
//
// Resource consisting of the result of running a CGI script. The constructor
// accepts a path to the script, a CGI object describing the interpreter
// to use to run the script, and various informations computed by the
// URI resolver that must be passed to the script according to CGI/1.1.
// The object then spawn the interpreter, collect its output and transmit
// it to the client.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

ResourceScript::ResourceScript(fs::filepath const & scriptname, std::string const & scripturi, std::string const & pathinfo, Configuration::CGI const & cgi)
  : Resource("script " + scriptname.getStdString()),
    scriptname_(scriptname),
    scripturi_(scripturi),
    pathinfo_(pathinfo),
    cgi_(cgi),
    errors_() {
}

//--------------------------------------------------------------
// Transmit the resource to the provided HttpResponse object.
//--------------------------------------------------------------

void ResourceScript::transmit(HttpResponse & response, HttpRequest const & request) {
    std::vector<std::string> args = buildArguments();
    std::vector<std::string> env = buildEnvironment(request);

    std::vector<char const *> buffer;
    for (std::string const & x: args) {
        buffer.push_back(x.c_str());
    }
    buffer.push_back(nullptr);
    size_t nenv = buffer.size();
    for (std::string const & x: env) {
        buffer.push_back(x.c_str());
    }
    buffer.push_back(nullptr);

    response.emitHeader(HttpHeader::ContentType, "text/plain; charset=utf-8");  // default, should be overriden by the script itself
    response.emitHeader(HttpHeader::LastModified, response.getResponseDate().to_http());
    response.emitHeader(HttpHeader::Expires, response.getResponseDate().to_http()); // expires immediately
    response.emitHeader(HttpHeader::CacheControl, "no-cache, no-store, must-revalidate");
    response.emitHeader(HttpHeader::Pragma, "no-cache");

    char const ** ptr = buffer.data();
    if (!runScript(response, request.getBody(), ptr, ptr + nenv)) {
        LOG_ERROR("Fork of " << cgi_.getInterpreter() << " failed.");
        response.emitEol();
        response.emitPage("Not enough resources to fork interpreter.");
        response.emitEol();
    }
    response.flush();

    string::split(errors_, '\n', 0, string::trim_right, [&] (std::string & line) {
        LOG_ERROR("[" << args[0] << "] " << line);
        return true;
    });
}

//--------------------------------------------------------------
// Build the argument list to pass to the external interpreter.
//--------------------------------------------------------------

std::vector<std::string> ResourceScript::buildArguments() const {
    std::vector<std::string> result;

    // By convention, the first argument must be the application
    // name. (Path stripped).

    result.push_back(cgi_.getInterpreter().getLastComponent().getStdString());

    // Append extra arguments from the configuration. We have to parse
    // the string as bash would to build the args array.(Except we
    // don't handle $variables and filename substitution.)

    auto escape = [] (int ch) {
        switch (ch) {
        case 'a':   return '\a';
        case 'b':   return '\b';
        case 'e':   return '\x1B';
        case 'f':   return '\f';
        case 'n':   return '\n';
        case 'r':   return '\r';
        case 't':   return '\t';
        case 'v':   return '\v';
        default:    return '\0';
        }
    };

    std::string const & extra = cgi_.getCmdLine();
    std::string buffer;
    int state = 0;

    size_t count = extra.size();
    for (size_t i = 0; i < count; i++) {
        char ch = extra[i], esc;
        switch (state) {
        case 0:
            if (ch == '"') {
                state = 3;
            } else if (ch == '\'') {
                state = 5;
            } else if (!isspace(ch)) {
                buffer.push_back(ch);
                state = 1;
            }
            break;
        case 1:
            if (ch == '\\') {
                state = 2;
            } else if (isspace(ch)) {
                result.push_back(buffer);
                buffer.clear();
                state = 0;
            } else {
                buffer.push_back(ch);
            }
            break;
        case 2:
            if (ch == ' ' || ch == '\'' || ch == '"' || ch == '\\') {
                buffer.push_back(ch);
            } else if ((esc = escape(ch)) != 0) {
                buffer.push_back(esc);
            } else {
                buffer.push_back('\\');
                buffer.push_back(ch);
            }
            state = 1;
            break;
        case 3:
            if (ch == '\\') {
                state = 4;
            } else if (ch == '"') {
                result.push_back(buffer);
                buffer.clear();
                state = 0;
            } else {
                buffer.push_back(ch);
            }
            break;
        case 4:
            if (ch == '"' || ch == '\\') {
                buffer.push_back(ch);
            } else if ((esc = escape(ch)) != 0) {
                buffer.push_back(esc);
            } else {
                buffer.push_back('\\');
                buffer.push_back(ch);
            }
            state = 3;
            break;
        case 5:
            if (ch == '\'') {
                result.push_back(buffer);
                buffer.clear();
                state = 0;
            } else {
                buffer.push_back(ch);
            }
            break;
        }
    }
    if (!buffer.empty()) {
        result.push_back(buffer);
    }

    // The last argument must be the filename containing the script
    // to run.

    result.push_back(scriptname_.getStdString());
    return result;
}

//--------------------------------------------------------------
// Build the list of environment variables to pass to the
// external interpreter.
//--------------------------------------------------------------

std::vector<std::string> ResourceScript::buildEnvironment(HttpRequest const & request) const {
    std::vector<std::string> result;

    // Helper function to append a key=value entry to the 
    // list of environment variables.

    auto add = [&result] (char const * varname, bool force, std::string const & value) {
        if (force || !value.empty()) {
            std::string env;
            env.append(varname);
            env.push_back('=');
            env.append(value);
            result.push_back(std::move(env));
        }
    };

    // If the request contains a body, add variables to indicate
    // it size and mime type.

    size_t size = request.getBody().getSize();
    if (size > 0) {
        add("CONTENT_LENGTH",   true,   std::to_string(size));
        add("CONTENT_TYPE",     false,  request.getHeaderValue(HttpHeader::ContentType));
    }

    // Add all the standard CGI variables.

    Configuration const & configuration = Zinc::getInstance().getConfiguration();
    fs::filepath root = fs::getCurrentDirectory();

    add("DOCUMENT_ROOT",        true,   root.getStdString());
    add("GATEWAY_INTERFACE",    true,   "CGI/1.1");
    add("HTTP_ACCEPT",          false,  request.getHeaderValue(HttpHeader::Accept));
    add("HTTP_ACCEPT_CHARSET",  false,  request.getHeaderValue(HttpHeader::AcceptCharset));
    add("HTTP_ACCEPT_ENCODING", false,  request.getHeaderValue(HttpHeader::AcceptEncoding));
    add("HTTP_ACCEPT_LANGUAGE", false,  request.getHeaderValue(HttpHeader::AcceptLanguage));
    add("HTTP_CONNECTION",      false,  request.getHeaderValue(HttpHeader::Connection));
    add("HTTP_COOKIE",          false,  request.getHeaderValue(HttpHeader::Cookie));
    add("HTTP_HOST",            false,  request.getHeaderValue(HttpHeader::Host));
    add("HTTP_REFERER",         false,  request.getHeaderValue(HttpHeader::Referer));
    add("HTTP_USER_AGENT",      false,  request.getHeaderValue(HttpHeader::UserAgent));
    add("HTTPS",                false,  request.isSecureHTTP() ? "on" : "");
    add("PATH_INFO",            true,   pathinfo_);
    add("QUERY_STRING",         true,   request.getURI().getQuery());
    add("REMOTE_ADDR",          true,   request.getRemoteAddress().getAddress());
    add("REMOTE_HOST",          true,   request.getRemoteAddress().getNameInfo());
    add("REMOTE_PORT",          true,   request.getRemoteAddress().getPort());
    add("REQUEST_METHOD",       true,   request.getVerb().getVerbName());
    add("REQUEST_URI",          true,   request.getURI().getRequestURI(false));
    add("SCRIPT_FILENAME",      true,   scriptname_.makeAbsolute().getStdString());
    add("SCRIPT_NAME",          true,   scripturi_);
    add("SERVER_ADDR",          true,   request.getLocalAddress().getAddress());
    add("SERVER_ADMIN",         false,  configuration.getServerAdmin());
    add("SERVER_NAME",          false,  configuration.getServerName());
    add("SERVER_PORT",          false,  request.getLocalAddress().getPort());
    add("SERVER_PROTOCOL",      true,   "HTTP/1.1");
    add("SERVER_SOFTWARE",      true,   Zinc::getInstance().getVersionString());

    size_t sep = scripturi_.rfind('/');
    std::string tmp = (sep != std::string::npos) ? scripturi_.substr(0, sep) : scripturi_;
    fs::filepath translated = root + fs::makeFilepathFromURI(tmp + pathinfo_);
    add("PATH_TRANSLATED", true, translated.getStdString());

    // Add PHP-specific variables.

    if (string::compare_i(cgi_.getSectionName(), "PHP")) {
        add("PHP_SELF",         true,   scripturi_);
        add("REDIRECT_STATUS",  true,   "204");
    }

    return result;
}

//--------------------------------------------------------------
// Spawn the external interpreter, passing arguments and environment
// variables, and redirecting its standard input/output.
//--------------------------------------------------------------

bool ResourceScript::runScript(HttpResponse & response, fs::tmpfile const & body, char const ** args, char const ** env) {

    for (size_t i = 0; args[i]; i++) {
        LOG_TRACE("execve arg: " << args[i]);
    }
    for (size_t i = 0; env[i]; i++) {
        LOG_TRACE("execve env: " << env[i]);
    }

#ifdef _WIN32

#else
    // Create pipes to redirect the interpreter standard and
    // error outputs, then fork.

    int stdout[2], stderr[2];
    if (pipe(stdout) < 0) {
        return false;
    }
    if (pipe(stderr) < 0) {
        close(stdout[0]);
        close(stdout[1]);
        return false;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(stdout[0]);
        close(stdout[1]);
        close(stderr[0]);
        close(stderr[1]);
        return false;
    }

    if (pid == 0) {

        // We are in the child process. Redirect the standard and error
        // output to our pipes, and close unused descriptors.

        dup2(stdout[1], STDOUT_FILENO);
        dup2(stderr[1], STDERR_FILENO);
        close(stdout[0]);
        close(stdout[1]);
        close(stderr[0]);
        close(stderr[1]);

        // If the request has a body, redirect the standard input to
        // the file containing the body content.

        if (body.getSize()) {
            dup2(body.getFileDescriptor(), STDIN_FILENO);
            lseek(STDIN_FILENO, 0, SEEK_SET);
        }

        // Execute the interpreter. If the call is successful, this
        // call never returns.

        execve(cgi_.getInterpreter().getCString(), const_cast<char **>(args), const_cast<char **>(env));

        // In case executing the interpreter fails, print an error
        // message and exit immediately.

        std::cerr << "Unable to execute interpreter " << cgi_.getInterpreter() << std::endl;
        _exit(EXIT_FAILURE);    // no destructor callings

    } else {

        // We are in the parent process. Close the unused end of our
        // pipes and loop to forward the standard output of the interpreter
        // to the client, and the error output to a local buffer.

        close(stdout[1]);
        close(stderr[1]);

        struct pollfd pf[2];
        pf[0].fd = stdout[0];
        pf[1].fd = stderr[0];
        char buffer[1024];

        for (int eof = 0; eof != 0x03; ) {
            pf[0].events = pf[1].events = POLLIN;
            int r = poll(pf, 2, 5000);
            if (r > 0) {
                if (pf[0].revents & (POLLIN | POLLHUP)) {
                    int count = read(stdout[0], buffer, sizeof(buffer));
                    if (count > 0) {
                        response.write(buffer, count);
                    } else {
                        eof |= 0x01;
                    }
                }
                if (pf[1].revents & (POLLIN | POLLHUP)) {
                    int count = read(stderr[0], buffer, sizeof(buffer));
                    if (count > 0) {
                        errors_.append(buffer, count);
                    } else {
                        eof |= 0x02;
                    }
                }
            } else if (r < 0) {
                LOG_ERROR("internal error: poll() failed");
                break;
            }
        }

        // Wait for the child process to terminate and close the last
        // descriptors.

        waitpid(pid, nullptr, 0);
        close(stdout[0]);
        close(stderr[0]);
    }
#endif
    return true;
}

//========================================================================
