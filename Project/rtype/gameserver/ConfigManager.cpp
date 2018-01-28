//
// Created by doom on 28/01/18.
//

#include <string>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <gameserver/ConfigManager.hpp>

void rtype::ConfigManager::_parseConfigInside(const rapidjson::Document &doc, const std::string &faction)
{
    const auto &boxCFG = doc["Box"];
    _factionToBounds.emplace(faction, sf::IntRect{boxCFG.GetObject()["position"].GetObject()["x"].GetInt(),
                                                  boxCFG.GetObject()["position"].GetObject()["y"].GetInt(),
                                                  boxCFG.GetObject()["size"].GetObject()["width"].GetInt(),
                                                  boxCFG.GetObject()["size"].GetObject()["height"].GetInt()});
}

bool rtype::ConfigManager::_loadFactionConfig(const std::string &faction) noexcept
{
    using namespace std::string_literals;
    try {
        const auto path = "assets/config/animations/" + faction + "Lv1AttackInGame.json";
        rapidjson::Document doc;
        std::ifstream ifs(path);
        rapidjson::IStreamWrapper isw(ifs);
        doc.ParseStream(isw);
        if (doc.HasParseError()) {
            throw std::runtime_error("Parse file -> "s + path + " failed."s);
        }
        _parseConfigInside(doc, faction);
    }
    catch (const std::exception &error) {
        return false;
    }
    return true;
}
