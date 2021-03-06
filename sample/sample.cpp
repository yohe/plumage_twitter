#include <iostream>
#include <fstream>
#include <functional>

#include <plumage/plugin_repository.hpp>
#include <plumage/plugin_manager.hpp>

#include "../picojson/picojson.h"
#include <boost/property_tree/ptree.hpp>

#include "plumage_twitter/plumage_twitter.hpp"

int usage() {
    std::cout << "Usage : tweet <option> <param>" << std::endl
              << "        Option : " << std::endl
              << "          [-s] : Search tweets. param is search string." << std::endl
              << "                 example : tweet -s \"vim script\"" << std::endl
              << "          [-t] : Tweeting. Param is tweet string you want to tweet." << std::endl
              << "                 example : tweet -t \" I like vim." << std::endl
              << "          [-r] : Retweet a tweet. Param is tweet ID you want to tweet." << std::endl
              << "                 example : tweet -r 12345" << std::endl
              << "          [-g] : Get the home timeline. Parameters of this option is optional," << std::endl
              << "                 and this option can specify one or two parameters." << std::endl
              << "                 First parameter is size you want to get the latest home timeline." << std::endl
              << "                 Second parameter is since_id you want to get the home timeline." << std::endl
              << "                     example : tweet -g 20" << std::endl
              << "                     example : tweet -g 20 1500" << std::endl
              << "          [-d] : Delete a tweet. Param is tweet ID you want to delete." << std::endl
              << "                 example : tweet -d 12345" << std::endl
              << "          [-S] : Display the realtime tweet. Param is nothing." << std::endl
              << "                 example : tweet -S" << std::endl;
    return 1;
}

std::string oauth_verifier_input(std::string url) {
    std::string cmd = "open " + url;
    system(cmd.c_str());

    std::cout << "PIN :";
    std::string PIN;
    std::cin >> PIN;
    return PIN;
}

size_t responseListner(char* data, size_t size) {
    std::stringstream ss;
    ss.write(data, size);

    picojson::value v;
    picojson::parse(v, ss);

    picojson::object obj = v.get<picojson::object>();
    if(v.contains("text")) {
        if(obj["lang"].to_str() == "ja") {
            picojson::object userObj = obj["user"].get<picojson::object>();
            std::cout << obj["id_str"].to_str() << " : " << userObj["name"].to_str() << " : " << obj["text"].to_str() << std::endl;
        }
    }
    return size;
}

void streamingSample(plumage::PluginInterface* pif, int argc, char const* argv[]) {
    std::function<size_t(char*,size_t)> f = responseListner;
    boost::any query(&f);
    pif->call("streamingSample", query);
}
void searchTweet(plumage::PluginInterface* pif, int argc, char const* argv[]) {
    std::string data = argv[2];
    boost::any query(data);
    boost::any ret = pif->call("searchTweet", query);
    picojson::value* v = boost::any_cast<picojson::value*>(ret);
    picojson::object obj = v->get<picojson::object>();
    if(!v->contains("statuses")) {
        std::cout << "search error." << std::endl;
        delete v;
        return;
    }
    picojson::array statuses = obj["statuses"].get<picojson::array>();
    picojson::array::iterator it;
    for (it = statuses.begin(); it != statuses.end(); it++) {
        picojson::object statusesObj = it->get<picojson::object>();
        picojson::object userObj = statusesObj["user"].get<picojson::object>();
        std::cout << statusesObj["id_str"].to_str() << " : " << userObj["name"].to_str() << " : " << statusesObj["text"].to_str() << std::endl;
    }
    delete v;
}
void tweet(plumage::PluginInterface* pif, int argc, char const* argv[]) {
    std::string data = argv[2];
    boost::any query(data);
    boost::any ret = pif->call("tweet", query);
    picojson::value* v = boost::any_cast<picojson::value*>(ret);
    if(!v->contains(std::string("text"))) {
        std::cout << "tweet error." << std::endl;
    }
    delete v;
}
void retweet(plumage::PluginInterface* pif, int argc, char const* argv[]) {
    std::string data = argv[2];
    if(std::find_if(data.begin(), data.end(), std::ptr_fun(isalpha)) != data.end()) {
        std::cout << "Tweet id parameter is invalid." << std::endl;
        return;
    }

    boost::any query(data);
    boost::any ret = pif->call("retweet", query);
    picojson::value* v = boost::any_cast<picojson::value*>(ret);
    if(!v->contains(std::string("text"))) {
        std::cout << "retweet error." << std::endl;
    }
    delete v;
}
void getHomeTimeline(plumage::PluginInterface* pif, int argc, char const* argv[]) {
    std::string data = "";
    if(argc == 2) {
        data = "20";
    } else {
        data = argv[2];
    }

    if(std::find_if(data.begin(), data.end(), std::ptr_fun(isalpha)) != data.end()) {
        std::cout << "Count parameter is invalid." << std::endl;
        return;
    }

    boost::any ret;
    if(argc <= 3) {
        boost::any query(data);
        ret = pif->call("getHomeTimeline", query);
    } else if(argc == 4) {
        std::string chk = argv[3];
        if(std::find_if(chk.begin(), chk.end(), std::ptr_fun(isalpha)) == chk.end()) {
            std::cout << "Since_id parameter is invalid." << std::endl;
            return;
        }

        boost::any query = std::make_tuple(data, std::string(argv[3]));
        ret = pif->call("getHomeTimeline", query);
    }
    picojson::value* v = boost::any_cast<picojson::value*>(ret);
    if(!v->is<picojson::array>()) {
        std::cout << "Home timeline get error." << std::endl;
        delete v;
        return;
    }
    picojson::array arr = v->get<picojson::array>();
    picojson::array::iterator it;
    for (it = arr.begin(); it != arr.end(); it++) {
        picojson::object statusesObj = it->get<picojson::object>();
        picojson::object userObj = statusesObj["user"].get<picojson::object>();
        std::cout << statusesObj["id_str"].to_str() << " : " << userObj["name"].to_str() << " : " << statusesObj["text"].to_str() << std::endl;
    }
    delete v;
}
void deleteTweet(plumage::PluginInterface* pif, int argc, char const* argv[]) {
    std::string data = argv[2];
    if(std::find_if(data.begin(), data.end(), std::ptr_fun(isalpha)) != data.end()) {
        std::cout << "Tweet id parameter is invalid." << std::endl;
        return;
    }
    boost::any query(data);
    boost::any ret = pif->call("deleteTweet", query);
    picojson::value* v = boost::any_cast<picojson::value*>(ret);
    if(!v->contains(std::string("text"))) {
        std::cout << "delete error." << std::endl;
    }
    delete v;
}

int main(int argc, char const* argv[])
{
    if(argc < 2) {
        return usage();
    }

    plumage::PluginManager manager;
    try {
#ifdef MAC_OSX
        manager.loadPlugin("../lib/libplumage_webapi.dylib", "createPlumageWebApiPlugin");
        manager.loadPlugin("../lib/libplumage_twitter.dylib", "createPlumageTwitterPlugin");
#else
        manager.loadPlugin("../lib/libplumage_webapi.so", "createPlumageWebApiPlugin");
        manager.loadPlugin("../lib/libplumage_twitter.so", "createPlumageTwitterPlugin");
#endif
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    plumage::PluginRepository* webApiRepos = manager.getPluginRepository("PlumageWebApi", 1, false);
    plumage::PluginRepository* twitterRepos = manager.getPluginRepository("PlumageTwitter", 1, false);
    if(webApiRepos == nullptr || twitterRepos == nullptr) {
        std::cout << "Repository is not found" << std::endl;
        return 0;
    }
    try {
        webApiRepos->activate();
        twitterRepos->activate();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    plumage::PluginInterface* pif = twitterRepos->getActivatedPlugin();
    if(pif == nullptr) {
        std::cout << "plugin is not found" << std::endl;
        return 0;
    }
    pif->start();

    try {
        boost::any param(std::make_tuple("consumer-key",
                                         "consumer-secret",
                                         "",
                                         ""));
        pif->call("setOAuthParam", param);

        std::function<std::string(std::string)> callbackFunc = oauth_verifier_input;
        boost::any callback(callbackFunc);
        pif->call("authenticate", callback);

        std::string option = argv[1];
        if(option != "-g" && option != "-S") {
            if(argc < 3) {
                return usage();
            }
        }
        if(option == "-s") {
            searchTweet(pif, argc, argv);
        } else if (option == "-t") {
            tweet(pif, argc, argv);
        } else if (option == "-r") {
            retweet(pif, argc, argv);
        } else if (option == "-g") {
            getHomeTimeline(pif, argc, argv);
        } else if (option == "-d") {
            deleteTweet(pif, argc, argv);
        } else if (option == "-S") {
            streamingSample(pif, argc, argv);
        }

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pif->stop();
    std::cout << "Success end." << std::endl;
    return 0;
}

