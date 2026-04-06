#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

HttpResponse&	checkPermissions(httpInfo& httpinfo, const std::string& filepath)
{
	//test root + uri
	struct stat	fileinfo;
	if (stat(filepath.c_str(), &fileinfo) == -1)
		return (returnResponse(httpinfo.res, 404, "Not Found", httpinfo.conf));

	//is it a directory?
	else if (S_ISDIR(fileinfo.st_mode))
		return (returnResponse(httpinfo.res, 403, "Forbidden: cannot DELETE a directory", httpinfo.conf));

	//user can write in parent dir?
	std::string parentDir = filepath.substr(0, filepath.rfind("/"));
	if (access(parentDir.c_str(), W_OK) == -1)
    	return (returnResponse(httpinfo.res, 403, "Forbidden", httpinfo.conf));

	return (httpinfo.res);
}

HttpResponse& deleteHandler(httpInfo& httpinfo)
{
	if (checkPermissions(httpinfo, httpinfo.filepath).getStatus() != 0)
		return (httpinfo.res);

	if (remove(httpinfo.filepath.c_str()) == 0)
	{
		httpinfo.res.setStatus(204);
		httpinfo.res.setPhrase("No Content: deleted succesfully");
		return (httpinfo.res);
	}
	else if (errno == EACCES)
		return (returnResponse(httpinfo.res, 403, "Forbidden", httpinfo.conf));
	else
		return (returnResponse(httpinfo.res, 500, "Internal Server Error", httpinfo.conf));
}
