#include "DockerClient.hpp"
#include "SimpleHttpClient.hpp"

#include <fstream>

namespace DockerClientpp {
class DockerClient::Impl {
 public:
  Impl(const SOCK_TYPE type, const string &path);
  ~Impl();
  void setAPIVersion(const string &api);
  string listImages();
  string createContainer(const json &config, const string &name = "");
  void startContainer(const string &identifier);
  void stopContainer(const string &identifier);
  void removeContainer(const string &identifier, bool remove_volume, bool force,
                       bool remove_link);
  string createExecution(const string &identifier, const json &config);
  string startExecution(const string &id, const json &config);
  string inspectExecution(const string &id);
  string getExecutionStats(const string &id);
  void downloadImage(const string &imageName, const string &tag, const json &config);
  ExecRet executeCommand(const string &identifier, const vector<string> &cmd);
  void putFiles(const string &identifier, const vector<string> &files,
                const string &path);
  void getFile(const string &identifier, const string &file,
               const string &path);

 private:
  Http::Header createCommonHeader(size_t content_length);

  Http::SimpleHttpClient http_client;
  string api_version;
};
}  // namespace DockerClientpp

using namespace DockerClientpp;
using namespace Http;
using namespace Utility;

DockerClient::Impl::Impl(const SOCK_TYPE type, const string &path)
    : http_client(type, path), api_version("v1.24") {}

DockerClient::Impl::~Impl() {}

void DockerClient::Impl::setAPIVersion(const string &api) {
  api_version = api;
}

string DockerClient::Impl::listImages() {
  Header header = createCommonHeader(0);
  Uri uri = "/images/json";
  shared_ptr<Response> res = http_client.Get(uri, header, {});
  switch (res->status_code) {
    case 200: {
      break;
    }
    default:
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
      break;
  }
  return res->body;
}

Http::Header DockerClient::Impl::createCommonHeader(size_t content_length) {
  return {
      {"Content-Type", "application/json"},
      {"Content-Length", std::to_string(content_length)},
      {"Host", api_version},
      {"Accept", "*/*"},
  };
}

string DockerClient::Impl::createContainer(const json &config,
                                           const string &name) {
  QueryParam query_param{};
  if (!name.empty()) {
    query_param["name"] = name;
  }

  string post_data = config.dump();
  Header header = createCommonHeader(post_data.size());
  Uri uri = "/containers/create";
  shared_ptr<Response> res =
      http_client.Post(uri, header, query_param, post_data);
  json body = json::parse(res->body);
  switch (res->status_code) {
    case 201:
      break;
    default:
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
  }
  return body["Id"];
}

void DockerClient::Impl::startContainer(const string &identifier) {
  Header header = createCommonHeader(0);
  Uri uri = "/containers/" + identifier + "/start";
  shared_ptr<Response> res = http_client.Post(uri, header, {}, "");
  switch (res->status_code) {
    case 204:
      break;
    default: {
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
    }
  }
}

void DockerClient::Impl::stopContainer(const string &identifier) {
  Header header = createCommonHeader(0);
  Uri uri = "/containers/" + identifier + "/stop";
  shared_ptr<Response> res = http_client.Post(uri, header, {}, "");
  switch (res->status_code) {
    case 204:
      break;
    default: {
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
    }
  }
}

void DockerClient::Impl::removeContainer(const string &identifier,
                                         bool remove_volume, bool force,
                                         bool remove_link) {
  Header header = createCommonHeader(0);
  Uri uri = "/containers/" + identifier;
  QueryParam query_param{{"v", std::to_string(remove_volume)},
                         {"force", std::to_string(force)},
                         {"link", std::to_string(remove_link)}};
  shared_ptr<Response> res = http_client.Delete(uri, header, query_param);
  switch (res->status_code) {
    case 204:
      break;
    default: {
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
    }
  }
}

string DockerClient::Impl::createExecution(const string &identifier,
                                           const json &config) {
  string post_data = config.dump();
  Header header = createCommonHeader(post_data.size());
  Uri uri = "/containers/" + identifier + "/exec";
  shared_ptr<Response> res = http_client.Post(uri, header, {}, post_data);
  json body = json::parse(res->body);
  switch (res->status_code) {
    case 201:
      break;
    default:
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
  }
  return body["Id"];
}

string DockerClient::Impl::startExecution(const string &id,
                                          const json &config) {
  string post_data = config.dump();
  Header header = createCommonHeader(post_data.size());
  Uri uri = "/exec/" + id + "/start";
  shared_ptr<Response> res = http_client.Post(uri, header, {}, post_data);
  switch (res->status_code) {
    case 200:
      break;
    default:
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
  }
  return res->body;
}


string DockerClient::Impl::getExecutionStats(const string &id){
  Header header = createCommonHeader(0);
  Uri uri = "/exec/" + id + "/stats";
  shared_ptr<Response> res = http_client.Get(uri, header, {});
  switch (res->status_code) {
    case 200:
      break;
    default:
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
  }
  return res->body;
}
//POST /v1.24/images/create?fromImage=busybox&tag=latest HTTP/1.1

void DockerClient::Impl::downloadImage(const string &imageName, const string &tag, const json &config){
  string post_data = config.dump();
  Header header = createCommonHeader(post_data.size());
  Uri uri = "/images/create";
  QueryParam query_param{{"fromImage", imageName}};
  if(!tag.empty()){
    query_param.insert({"tag",tag});
  }
  shared_ptr<Response> res =
      http_client.Post(uri, header, query_param, post_data);
  json body = json::parse(res->body);

  switch (res->status_code) {
    case 204:
      break;
    default: {
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
    }
  }
   
}

string DockerClient::Impl::inspectExecution(const string &id) {
  Header header = createCommonHeader(0);
  Uri uri = "/exec/" + id + "/json";
  shared_ptr<Response> res = http_client.Get(uri, header, {});
  switch (res->status_code) {
    case 200:
      break;
    default:
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
  }
  return res->body;
}

ExecRet DockerClient::Impl::executeCommand(const string &identifier,
                                           const vector<string> &cmd) {
  string id = this->createExecution(identifier, {{"AttachStdout", true},
                                                 {"AttachStderr", true},
                                                 {"Tty", false},
                                                 {"Cmd", cmd}});
  ExecRet ret;
  ret.output = this->startExecution(id, {{"Detach", false}, {"Tty", false}});
  json status = json::parse(this->inspectExecution(id));
  ret.ret_code = status["ExitCode"].get<int>();
  return ret;
}

void DockerClient::Impl::putFiles(const string &identifier,
                                  const vector<string> &files,
                                  const string &path) {
  Utility::Archive ar;
  ar.addFiles(files);
  string put_data = ar.getTar();
  Header header = createCommonHeader(put_data.size());
  Uri uri = "/containers/" + identifier + "/archive";
  header["Content-Type"] = "application/x-tar";
  QueryParam query_param{{"path", path}};
  shared_ptr<Response> res =
      http_client.Put(uri, header, query_param, put_data);
  switch (res->status_code) {
    case 200:
      break;
    default:
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
  }
}

void DockerClient::Impl::getFile(const string &identifier, const string &file,
                                 const string &path) {
  Header header = createCommonHeader(0);
  Uri uri = "/containers/" + identifier + "/archive";
  shared_ptr<Response> res = http_client.Get(uri, header, {{"path", file}});
  switch (res->status_code) {
    case 200:
      break;
    default:
      json body = json::parse(res->body);
      throw DockerOperationError(uri, res->status_code,
                                 body["message"].get<string>());
  }

  Utility::Archive::extractTar(res->body, path);
}

//-------------------------DockerClient Implementation-------------------------

DockerClient::DockerClient(const SOCK_TYPE type, const string &path)
    : m_impl(new Impl(type, path)) {}

DockerClient::~DockerClient() {}

void DockerClient::setAPIVersion(const string &api) {
  m_impl->setAPIVersion(api);
}

string DockerClient::listImages() {
  return m_impl->listImages();
}

string DockerClient::createContainer(const json &config, const string &name) {
  return m_impl->createContainer(config, name);
}

string DockerClient::getExecutionStats(const std::string &id){
  return m_impl->getExecutionStats(id);
}

void DockerClient::downloadImage(const string &imageName, const string &tag, const json &config){
  m_impl->downloadImage(imageName,tag,config);
}


void DockerClient::startContainer(const string &identifier) {
  m_impl->startContainer(identifier);
}

void DockerClient::stopContainer(const string &identifier) {
  m_impl->stopContainer(identifier);
}

void DockerClient::removeContainer(const string &identifier, bool remove_volume,
                                   bool force, bool remove_link) {
  m_impl->removeContainer(identifier, remove_volume, force, remove_link);
}

string DockerClient::createExecution(const string &identifier,
                                     const json &config) {
  return m_impl->createExecution(identifier, config);
}

string DockerClient::startExecution(const string &id, const json &config) {
  return m_impl->startExecution(id, config);
}

string DockerClient::inspectExecution(const string &id) {
  return m_impl->inspectExecution(id);
}

ExecRet DockerClient::executeCommand(const string &identifier,
                                     const vector<string> &cmd) {
  return m_impl->executeCommand(identifier, cmd);
}

void DockerClient::putFiles(const string &identifier,
                            const vector<string> &files, const string &path) {
  m_impl->putFiles(identifier, files, path);
}

void DockerClient::getFile(const string &identifier, const string &file,
                           const string &path) {
  m_impl->getFile(identifier, file, path);
}
