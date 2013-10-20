
#include <tuple>
#include <iostream>
#include <ostream>
#include <fstream>
#include <utility>
#include <functional>
#include <openssl/hmac.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "plumage_twitter/plumage_twitter.hpp"

extern "C" plumage::PluginHolder* createPlumageTwitterPlugin() {
    PlumageTwitter* pif = new PlumageTwitter();
    plumage::PluginHolder* holder = new plumage::PluginHolder(pif);
    return holder;
}

void PlumageTwitter::init() {
    methodList_["setOAuthParam"] = &PlumageTwitter::setOAuthParam;
    methodList_["authenticate"] = &PlumageTwitter::authenticate;
    methodList_["searchTweet"] = &PlumageTwitter::searchTweet;
    methodList_["tweet"] = &PlumageTwitter::tweet;
    methodList_["retweet"] = &PlumageTwitter::retweet;
    methodList_["deleteTweet"] = &PlumageTwitter::deleteTweet;
    methodList_["getHomeTimeline"] = &PlumageTwitter::getHomeTimeline;

    methodList_["streamingSample"] = &PlumageTwitter::streamingSample;
    methodList_["streamingFilter"] = &PlumageTwitter::streamingFilter;
}

bool PlumageTwitter::doStart() {
    webapi_ = getRequiredPlugin("PlumageWebApi");
    boost::any ret = webapi_->call("createHandle");
    handle_ = boost::any_cast<CURL*>(ret);
    return true;
}
bool PlumageTwitter::doStop() {
    if(oauthHandle_ != nullptr) {
        boost::any delParam(oauthHandle_);
        webapi_->call("deleteOAuthHandle", delParam);
        oauthHandle_ = nullptr;
    }
    boost::any p(handle_);
    webapi_->call("deleteHandle", p);
    return true;
}

bool PlumageTwitter::isCompatible(int interfaceVersion) const {
    if(getInterfaceVersion() == interfaceVersion) {
        return true;
    }
    return false;
}

bool PlumageTwitter::isCallable(const std::string& methodName) const {
    if(methodList_.count(methodName) == 0) {
        return false;
    }
    return true;
}

boost::any PlumageTwitter::doCall(std::string methodName, boost::any& parameter) throw (std::exception) {
    if(!isStarted()) {
        throw std::runtime_error("PlumageTwitter don't start.");
    }

    Method method = methodList_.at(methodName);
    return (this->*method)(parameter);
}

boost::any PlumageTwitter::searchTweet(boost::any& parameter) {
    if(oauthHandle_ == nullptr) {
        throw std::logic_error("PlumageWebApi : OAuth error.");
    }
    if(parameter.type() != typeid(std::string)) {
        throw std::logic_error("PlumageTwitter::searchTweet : parameter invalid.");
    }

    std::stringstream ss;
    std::string data = boost::any_cast<std::string>(parameter);
    std::string query;
    query = "q=" + data + "&lang=ja&local=ja";
    boost::any param(std::make_tuple(handle_, oauthHandle_, "https://api.twitter.com/1.1/search/tweets.json", query.c_str(), (std::ostream*)&ss));
    webapi_->call("getOnOAuth", param);
    boost::any searchResult((std::istream*)&ss);
    return std::move(webapi_->call("parseJsonData", searchResult));
}
boost::any PlumageTwitter::tweet(boost::any& parameter) {
    if(oauthHandle_ == nullptr) {
        throw std::logic_error("PlumageTwitter : OAuth error.");
    }
    if(parameter.type() != typeid(std::string)) {
        throw std::logic_error("PlumageTwitter::tweet : parameter invalid.");
    }

    std::stringstream ss;
    std::string data = boost::any_cast<std::string>(parameter);
    std::string query;
    query = "status=" + data + "&lang=ja&local=ja";
    boost::any param(std::make_tuple(handle_, oauthHandle_, "https://api.twitter.com/1.1/statuses/update.json", query.c_str(), (std::ostream*)&ss));
    webapi_->call("postOnOAuth", param);
    boost::any searchResult((std::istream*)&ss);
    return std::move(webapi_->call("parseJsonData", searchResult));
}
boost::any PlumageTwitter::retweet(boost::any& parameter) {
    if(oauthHandle_ == nullptr) {
        throw std::logic_error("PlumageTwitter : OAuth error.");
    }
    if(parameter.type() != typeid(std::string)) {
        throw std::logic_error("PlumageTwitter::retweet : parameter invalid.");
    }

    std::stringstream ss;
    std::string id = boost::any_cast<std::string>(parameter);
    std::string url = "https://api.twitter.com/1.1/statuses/retweet/" + id + ".json";
    boost::any param(std::make_tuple(handle_, oauthHandle_, url.c_str(), "", (std::ostream*)&ss));
    webapi_->call("postOnOAuth", param);
    boost::any searchResult((std::istream*)&ss);
    return std::move(webapi_->call("parseJsonData", searchResult));
}
boost::any PlumageTwitter::deleteTweet(boost::any& parameter) {
    if(oauthHandle_ == nullptr) {
        throw std::logic_error("PlumageTwitter : OAuth error.");
    }
    if(parameter.type() != typeid(std::string)) {
        throw std::logic_error("PlumageTwitter::deleteTweet : parameter invalid.");
    }

    std::stringstream ss;
    std::string id = boost::any_cast<std::string>(parameter);
    std::string url = "https://api.twitter.com/1.1/statuses/destroy/" + id + ".json";
    boost::any param(std::make_tuple(handle_, oauthHandle_, url.c_str(), "", (std::ostream*)&ss));
    webapi_->call("postOnOAuth", param);
    boost::any searchResult((std::istream*)&ss);
    return std::move(webapi_->call("parseJsonData", searchResult));
}
boost::any PlumageTwitter::getHomeTimeline(boost::any& parameter) {
    if(oauthHandle_ == nullptr) {
        throw std::logic_error("PlumageTwitter : OAuth error.");
    }
    std::string since = "0";
    std::string count = "5";

    std::stringstream data;
    if(parameter.type() == typeid(std::string)) {
        count = boost::any_cast<std::string>(parameter);
        data << "count=" << count;
    } else if (parameter.type() == typeid(std::tuple<std::string, std::string>)) {
        std::tuple<std::string, std::string> p = boost::any_cast<std::tuple<std::string, std::string>>(parameter);
        count = std::get<0>(p);
        since = std::get<1>(p);
        data << "since_id=" << since;
        data << "&count=" << count;
    } else {
        throw std::logic_error("PlumageTwitter::getHomeTimeline : parameter invalid.");
    }

    std::stringstream ss;
    std::string url = "https://api.twitter.com/1.1/statuses/home_timeline.json";
    boost::any param(std::make_tuple(handle_, oauthHandle_, url.c_str(), data.str().c_str(), (std::ostream*)&ss));
    webapi_->call("getOnOAuth", param);
    boost::any searchResult((std::istream*)&ss);
    return std::move(webapi_->call("parseJsonData", searchResult));
}


boost::any PlumageTwitter::setOAuthParam(boost::any& parameter) {
    if(parameter.type() != typeid(std::tuple<const char*, const char*, const char*, const char*>)) {
        throw std::logic_error("PlumageTwitter::searchTweet : parameter invalid.");
    }

    if(oauthHandle_ != nullptr) {
        std::tuple<const char*, const char*, const char*, const char*> p =
            boost::any_cast<std::tuple<const char*, const char*, const char*, const char*>>(parameter);

        boost::any param = std::make_tuple(oauthHandle_, std::get<0>(p), std::get<1>(p), std::get<2>(p), std::get<3>(p));
        boost::any ret = webapi_->call("updateOAuthHandle", param);
        return nullptr;
    }

    boost::any ret = webapi_->call("createOAuthHandle", parameter);
    oauthHandle_ = boost::any_cast<void*>(ret);
    return nullptr;
}

boost::any PlumageTwitter::authenticate(boost::any& parameter) {

    if(parameter.type() != typeid(std::function<std::string(std::string)>)) {
        throw std::logic_error("PlumageTwitter::authenticate : parameter invalid.");
    }

    std::ifstream ifs(".oauth.conf");
    if(ifs) {
        std::string oauth_token;
        std::string oauth_token_secret;
        ifs >> oauth_token;
        ifs >> oauth_token_secret;
        if(!oauth_token.empty() && !oauth_token_secret.empty()) {
            boost::any param = std::make_tuple("", "", oauth_token.c_str(), oauth_token_secret.c_str());
            setOAuthParam(param);
            return nullptr;
        }
    }

    std::function<std::string(std::string)> callbackFunc = boost::any_cast<std::function<std::string(std::string)>>(parameter);

    boost::any param = std::make_tuple(handle_, oauthHandle_, "https://api.twitter.com/oauth/authorize", "https://api.twitter.com/oauth/request_token");
    boost::any ret = webapi_->call("getAuthorizeUrlOnOAuth", param);
    std::string url = boost::any_cast<std::string>(ret);
    std::string PIN = callbackFunc(url);

    param = std::make_tuple(handle_, oauthHandle_, "https://api.twitter.com/oauth/access_token", PIN.c_str());
    ret = webapi_->call("getAccessTokenOnOAuth", param);

    std::ofstream ofs(".oauth.conf");
    if(!ofs) {
        return nullptr;
    }

    std::map<std::string, std::string> accessInfo = boost::any_cast<std::map<std::string, std::string>>(ret);
    ofs << accessInfo["oauth_token"] << " " << accessInfo["oauth_token_secret"];

    return nullptr;
}

boost::any PlumageTwitter::streamingSample(boost::any& parameter) {
    if(oauthHandle_ == nullptr) {
        throw std::logic_error("PlumageTwitter : OAuth error.");
    }

    std::stringstream ss;
    std::function<size_t(char*, size_t)>* f = boost::any_cast<std::function<size_t(char*,size_t)>*>(parameter);

    std::string url = "https://stream.twitter.com/1.1/statuses/sample.json";
    boost::any param(std::make_tuple(handle_, oauthHandle_, url.c_str(), "", f));
    webapi_->call("getOnOAuth", param);
    //boost::any searchResult((std::istream*)&ss);
    //return std::move(webapi_->call("parseJsonData", searchResult));
    return nullptr;
}
boost::any PlumageTwitter::streamingFilter(boost::any& parameter) {
    if(oauthHandle_ == nullptr) {
        throw std::logic_error("PlumageTwitter : OAuth error.");
    }

    std::stringstream ss;
    std::string id = boost::any_cast<std::string>(parameter);
    std::string url = "https://stream.twitter.com/1.1/statuses/sample.json";
    boost::any param(std::make_tuple(handle_, oauthHandle_, url.c_str(), "", (std::ostream*)&std::cout));
    webapi_->call("getOnOAuth", param);
    //boost::any searchResult((std::istream*)&ss);
    //return std::move(webapi_->call("parseJsonData", searchResult));
    return nullptr;
}
