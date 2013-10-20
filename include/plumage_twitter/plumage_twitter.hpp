
#ifndef PLUMAGE_WEB_API_HPP
#define PLUMAGE_WEB_API_HPP

#include <cstdlib>
#include <set>
#include <string>
#include <sstream>

#include <curl/curl.h>

#include <plumage/plugin_entity.hpp>

class PlumageTwitter : public plumage::PluginEntity {

    const int INTERFACE_VERSION = 1;
    const int PLUGIN_VERSION = 1;

    typedef boost::any (PlumageTwitter::*Method)(boost::any&);

    CURL* handle_;
    void* oauthHandle_;
    plumage::PluginInterface* webapi_;
    std::map<std::string, Method> methodList_;

    void init();

public:
    PlumageTwitter() : plumage::PluginEntity("PlumageTwitter"), handle_(nullptr), oauthHandle_(nullptr) {
        init();
        requirement_.addRequirement("PlumageWebApi", 1);
    }

    virtual ~PlumageTwitter() { }

    virtual int getInterfaceVersion() const {
        return INTERFACE_VERSION;
    }
    virtual int getPluginVersion() const {
        return PLUGIN_VERSION;
    }
    virtual bool isDebug() const {
#ifdef DEBUG
        return true;
#else
        return false;
#endif
    }

    virtual bool isCompatible(int interfaceVersion) const;
    virtual bool isCallable(const std::string& methodName) const;

protected:
    // REST API
    boost::any searchTweet(boost::any& parameter);
    boost::any tweet(boost::any& parameter);
    boost::any retweet(boost::any& parameter);
    boost::any deleteTweet(boost::any& parameter);
    boost::any setOAuthParam(boost::any& parameter);
    boost::any authenticate(boost::any& parameter);
    boost::any getHomeTimeline(boost::any& parameter);

    // Streaming API
    boost::any streamingSample(boost::any& parameter);
    boost::any streamingFilter(boost::any& parameter);

private:
    virtual boost::any doCall(std::string methodName, boost::any& paramter) throw (std::exception);
    virtual bool doStart();
    virtual bool doStop();

};

#endif
