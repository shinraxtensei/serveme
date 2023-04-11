#include "../inc/servme.hpp"



std::string generateSessionId(size_t length) {
  static const char charset[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const size_t charset_size = sizeof(charset) - 1;
  std::srand(std::time(0));
  std::stringstream ss;
  for (size_t i = 0; i < length; ++i) {
    ss << charset[std::rand() % charset_size];
  }
  return ss.str();
}


std::string GetFutureTime()
{
    const std::time_t now = std::time(0);
    std::time_t future = now + TIMEOUT; 
    std::tm* timeinfo = std::localtime(&future);
    char buffer[80];

    std::strftime(buffer, 80, "Expires: %a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return std::string(buffer);
}

void Client::handleCookies()
{
    std::map<std::string, std::string>::iterator it;
    std::map<std::string, std::string>::iterator ite;
    it = this->request->headers.equal_range("Cookie:").first ;
    ite = this->request->headers.equal_range("Cookie:").second ;

    std::string session_id = "";
    std::string user_id = "";
    for (; it!= ite ; it++)
    {
        std::string cookie = it->second;
        if (cookie.find("session_id") != std::string::npos)
        {
            session_id = cookie.substr(cookie.find("session_id=") + 11  );
            session_id = session_id.substr(0, session_id.find_first_of(";") + 1);
            // std::cout << "session_id: " << session_id << std::endl;
        }
        if (cookie.find("user_id=") != std::string::npos)
        {

            user_id = cookie.substr(cookie.find("user_id=") + 8 );
            std::cout << "user_id: " << user_id << std::endl;
        }

        if (this->session.session_id == session_id && this->session.user_id == user_id)
        {
            //good
            return;
        }
    }

}