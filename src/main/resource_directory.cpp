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

#include <cassert>
#include <algorithm>

#include "zinc.h"
#include "resource_page_directory.h"
#include "resource_directory.h"

//========================================================================
// ResourceDirectory
//
// Resource consisting of a server-generated page containing a directory
// listing. Besides the path of the directory to list, the constructor
// accepts a map containing the arguments passed in the URI.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

ResourceDirectory::ResourceDirectory(std::string const & uri, std::unordered_map<std::string, std::string> const & query)
  : Resource("directory index for " + uri),
    uri_(uri),
    query_(query) {

    // Normalize the URI: no ending slash, except for
    // the root folder.

    assert(uri_.length() > 0);
    assert(uri_.front() == '/');

    while (uri_.length() > 1 && uri_.back() == '/') {
        uri_.pop_back();
    }
}

//--------------------------------------------------------------
// Transmit the resource to the provided HttpResponse object.
//--------------------------------------------------------------

void ResourceDirectory::transmit(HttpResponse & response, HttpRequest const & request) {

    // Emit the header.

    response.emitHeader(HttpHeader::ContentType, "text/html; charset=UTF-8");
    response.emitHeader(HttpHeader::LastModified, response.getResponseDate().to_http());
    response.emitHeader(HttpHeader::Expires, response.getResponseDate().to_http());   // expires immediately
    response.emitHeader(HttpHeader::CacheControl, "no-cache, no-store, must-revalidate");
    response.emitHeader(HttpHeader::Pragma, "no-cache");

    response.emitEol();

    // Helper structures to sort the listing by file name,
    // file size, or last modification date.

    static struct {
        int     order;
        bool (* compare)(fs::dirent const &, fs::dirent const &);
        struct {
            int link;
            int arrow;
        } fields[3];
    } orderDescription[] = {
        { 'N', [] (fs::dirent const & lhs, fs::dirent const & rhs) { return compareByName(lhs, rhs) < 0; }, {{ 'O',  1 }, { 'D',  0 }, { 'S',  0 }} },
        { 'O', [] (fs::dirent const & lhs, fs::dirent const & rhs) { return compareByName(lhs, rhs) > 0; }, {{ 'N', -1 }, { 'D',  0 }, { 'S',  0 }} },
        { 'D', [] (fs::dirent const & lhs, fs::dirent const & rhs) { return compareByDate(lhs, rhs) < 0; }, {{ 'N',  0 }, { 'E',  1 }, { 'S',  0 }} },
        { 'E', [] (fs::dirent const & lhs, fs::dirent const & rhs) { return compareByDate(lhs, rhs) > 0; }, {{ 'N',  0 }, { 'D', -1 }, { 'S',  0 }} },
        { 'S', [] (fs::dirent const & lhs, fs::dirent const & rhs) { return compareBySize(lhs, rhs) < 0; }, {{ 'N',  0 }, { 'D',  0 }, { 'T',  1 }} },
        { 'T', [] (fs::dirent const & lhs, fs::dirent const & rhs) { return compareBySize(lhs, rhs) > 0; }, {{ 'N',  0 }, { 'D',  0 }, { 'S', -1 }} },
    };

    static char const * columnNames[3] = {
        "Name", "Modification", "Size"
    };

    // Determine the sort order from the query string.

    int order = 0;
    std::string const & param = getParameterValue("S");
    if (param.length() == 1) {
        for (size_t i = 0; i < sizeof(orderDescription) / sizeof(orderDescription[0]); i++) {
            if (orderDescription[i].order == param.front()) {
                order = i;
                break;
            }
        }
    }

    // Generate and transmit the page.

    response.emitPage(reinterpret_cast<char const *>(page_directory_html), [&] (std::string const & field) {
        std::string ret;
        if (field == "server_version") {
            ret = string::encodeHtml(Zinc::getInstance().getVersionString());
        } else if (field == "server_name") {
            ret = string::encodeHtml(Zinc::getInstance().getConfiguration().getServerName());
        } else if (field == "server_addr") {
            ret = request.getLocalAddress().getAddress();
        } else if (field == "server_port") {
            ret = request.getLocalAddress().getPort();
        } else if (field == "folder") {
        	ret = string::encodeHtml(this->uri_);
		} else if (field == "content") {
            std::vector<fs::dirent> list;
            fs::makeFilepathFromURI(this->uri_).getDirectoryContent([&] (fs::dirent const & entry) {
                list.push_back(entry);
            });
            std::sort(list.begin(), list.end(), orderDescription[order].compare);

            std::string root = this->uri_;
            if (root.length() > 1) {
                root.push_back('/');
            }

            ret.reserve(1000 + 200 * list.size());
            ret += "<tr>";
            for (size_t i = 0; i < 3; i++) {
                ret += i ? "<th>" : "<th colspan=\"2\">";
                ret += "<a href=\"" + string::encodeHtml(root) + "?S=" + static_cast<char>(orderDescription[order].fields[i].link) + "\">" + columnNames[i] + "</a>";
                if (orderDescription[order].fields[i].arrow < 0) {
                    ret += " <img src=\"/__zinc__/arrow_down.png\" class=\"sort\" alt=\"sort descending\"/>";
                } else if (orderDescription[order].fields[i].arrow > 0) {
                    ret += " <img src=\"/__zinc__/arrow_up.png\" class=\"sort\" alt=\"sort ascending\"/>";
                }
                ret += "</th>";
            }
            ret += "</tr>\n";

            if (root.length() > 1) {
                ret += "<tr>";
                ret += "<td><img src=\"/__zinc__/back.png\" alt=\"folder icon\"/></td>";
                ret += "<td><a href=\"" + string::encodeHtml(root.substr(0, root.rfind('/', root.length() - 2) + 1)) + "\">Parent directory</a></td>";
                ret += "<td>&nbsp;</td>";
                ret += "<td>&nbsp;</td>";
                ret += "</tr>\n";
            }

            for (fs::dirent & entry: list) {
                ret += "<tr>";
                std::string link = root + entry.getName(), size;
                if (entry.getFileType() == fs::directory) {
                    ret += "<td class=\"col-icon\"><img src=\"/__zinc__/folder.png\" alt=\"folder icon\"/></td>";
                    link.push_back('/');
                    size = "&nbsp;";
                } else {
                    ret += "<td class=\"col-icon\"><img src=\"/__zinc__/document.png\" alt=\"document icon\"/></td>";
                    size = std::to_string(entry.getSize());
                }
                ret += "<td class=\"col-name\"><a href=\"" + string::encodeHtml(link) + "\">" + string::encodeHtml(entry.getName()) + "</a></td>";
                ret += "<td class=\"col-modif\">" + string::encodeHtml(formatModificationDate(entry)) + "</td>";
                ret += "<td class=\"col-size\">" + size + "</td>";
                ret += "</tr>\n";
            }
        }
        return ret;
    });

    response.flush();
}

//--------------------------------------------------------------
// Return the value of a parameter of the query string, or an
// empty string if this parameter is missing.
//--------------------------------------------------------------

std::string const & ResourceDirectory::getParameterValue(std::string const & name) const {
    auto got = query_.find(name);
    return got != query_.end() ? got->second : string::empty;
}

//--------------------------------------------------------------
// Format the modification date of a directory entry to a string.
//--------------------------------------------------------------

std::string ResourceDirectory::formatModificationDate(fs::dirent const & entry) const {
    return entry.getModificationDate().format("%Y-%m-%d %H:%M:%S", date::local);
}

//========================================================================
