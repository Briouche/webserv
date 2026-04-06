#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

static std::string	succesfullUploadBody()
{
	std::string body = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Upload</title><style>"
    "body { font-family: Arial, sans-serif; background: #1e1e1e; color: #f0f0f0; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }"
    ".card { background: #2a2a2a; border-radius: 16px; padding: 40px 60px; text-align: center; box-shadow: 0 0 30px rgba(100, 200, 255, 0.2); }"
    "h1 { color: #6cf; } a { color: #6cf; text-decoration: none; font-size: 1.2em; }"
    "a:hover { text-decoration: underline; }"
    "</style></head><body>"
    "<div class='card'>"
    "<div style='font-size: 4em'>&#x2705;</div>"
    "<h1>File uploaded successfully!</h1>"
    "<p>Your file has been saved on the server.</p>"
    "<a href='/'>&#8592; Go back</a>"
    "</div></body></html>";
	return (body);
}

HttpResponse&	uploadFile(httpInfo& httpinfo)
{
	const std::string& reqBody = httpinfo.req.getBody();
	
	// if upload_path in config, use this rather than uri dir
	if (!httpinfo.loc->upload_path.empty())
		httpinfo.filepath = httpinfo.loc->upload_path + httpinfo.filepath.substr(httpinfo.filepath.rfind("/"));

    std::string dirPath = httpinfo.filepath.substr(0, httpinfo.filepath.rfind('/'));
    // Check if directory exists, if not, create it
    struct stat st;
    if (stat(dirPath.c_str(), &st) != 0)
    {
        if (mkdir(dirPath.c_str(), 0755) != 0 && errno != EEXIST)
            return returnResponse(httpinfo.res, 500, "Internal Server Error: could not create directory", httpinfo.conf);
    }
    else if (!S_ISDIR(st.st_mode))
        return returnResponse(httpinfo.res, 500, "Internal Server Error: path exists but is not a directory", httpinfo.conf);

	else if (access(httpinfo.filepath.c_str(), F_OK) == 0)
    	return (returnResponse(httpinfo.res, 409, "Conflict: file already exists", httpinfo.conf));

	std::ofstream outfile(httpinfo.filepath.c_str(), std::ios::binary);
	if (!outfile.is_open())
		return (returnResponse(httpinfo.res, 500, "Internal Server Error: could not open file", httpinfo.conf));
	if (reqBody.empty())
		return (returnResponse(httpinfo.res, 204, "No content", httpinfo.conf));
	outfile.write(reqBody.c_str(), reqBody.size());
	if (outfile.fail())
	{
		outfile.close();
		return (returnResponse(httpinfo.res, 500, "Internal Server Error: write failed", httpinfo.conf));
	}

	outfile.close();
	httpinfo.res.setStatus(201);
	httpinfo.res.setPhrase("Created");
	httpinfo.res.addHeader("Content-Location", httpinfo.req.getUri());

	std::string body = succesfullUploadBody();
	httpinfo.res.setBody(body);
	std::ostringstream	bodySize;
	bodySize << body.size();
	httpinfo.res.addHeader("content-length", bodySize.str());
	httpinfo.res.addHeader("content-type", "text/html");
	return (httpinfo.res);
}

HttpResponse&	postHandler(httpInfo& httpinfo)
{
	if (isCgi(httpinfo.filepath))
	{
		std::string extention = validateCgi(httpinfo);				// get extention and check if its usable
		if (extention.empty())
			return (httpinfo.res);
		return (execCgi(httpinfo, extention));
	}
	else
		return (uploadFile(httpinfo));
}
